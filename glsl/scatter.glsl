#version 330 core

/*
The following interface is implemented in this shader:
//***** begin interface of scatter.glsl ***********************************
vec3 calculate_scattering(
vec3 start, // the start of the ray (the camera position)
vec3 dir, // the direction of the ray (the camera vector)
float max_dist, // the maximum distance the ray can travel (because something is in the way, like an object)

mat4 viewMatrix, // HACK we are using the camera orientation to position the planet correctly, since the given start position and direction are in camera space

vec3 scene_color, // the color of the scene
vec3 light_dir, // the direction of the light
vec3 light_intensity// how bright the light is, affects the brightness of the atmosphere
);
//***** end interface of scatter.glsl ***********************************
*/

vec3 rotate_vector(vec4 quat, vec3 vec) {
    return vec + 2.0 * cross(cross(vec, quat.xyz) + quat.w * vec, quat.xyz);
}

// https://www.shadertoy.com/view/wlBXWK
vec3 calculate_scattering(
vec3 start, // the start of the ray (the camera position)
vec3 dir, // the direction of the ray (the camera vector)
float max_dist, // the maximum distance the ray can travel (because something is in the way, like an object)

mat4 viewMatrix, // HACK we are using the camera orientation to position the planet correctly, since the given start position and direction are in camera space

vec3 scene_color, // the color of the scene
vec3 light_dir, // the direction of the light
vec3 light_intensity// how bright the light is, affects the brightness of the atmosphere
) {
    float planet_radius = 6371e3;// the radius of the planet
    float atmo_radius = 6471e3;// the radius of the atmosphere
    vec3 planet_position = vec3(0.0, -(planet_radius+100), 0.0);// the position of the planet

    vec3 beta_ray = vec3(5.5e-6, 13.0e-6, 22.4e-6);// the amount rayleigh scattering scatters the colors (for earth: causes the blue atmosphere)
    vec3 beta_mie = vec3(21e-6);// the amount mie scattering scatters colors
    vec3 beta_absorption = vec3(2.04e-5, 4.97e-5, 1.95e-6);// how much air is absorbed
    vec3 beta_ambient = vec3(0.0, 0.0, 0.0);// the amount of scattering that always occurs, can help make the back side of the atmosphere a bit brighter
    float g = 0.76;// the direction mie scatters the light in (like a cone). closer to -1 means more towards a single direction

    float height_ray = 8e3;// how high do you have to go before there is no rayleigh scattering?
    float height_mie = 1.2e3;// the same, but for mie
    float height_absorption = 30e3;// the height at which the most absorption happens
    float absorption_falloff = 4e3;// how fast the absorption falls off from the absorption height

    int steps_primary = 64;// the amount of steps along the 'primary' ray, more looks better but slower
    int steps_light = 8;// the amount of steps along the light ray, more looks better but slower

    planet_position = (viewMatrix * vec4(planet_position, 1.0)).xyz;

    // add an offset to the camera position, so that the atmosphere is in the correct position
    start -= planet_position;
    // calculate the start and end position of the ray, as a distance along the ray
    // we do this with a ray sphere intersect
    float a = dot(dir, dir);
    float b = 2.0 * dot(dir, start);
    float c = dot(start, start) - (atmo_radius * atmo_radius);
    float d = (b * b) - 4.0 * a * c;

    // stop early if there is no intersect
    if (d < 0.0) {
        return scene_color;
    }

    // calculate the ray length
    vec2 ray_length = vec2(
    max((-b - sqrt(d)) / (2.0 * a), 0.0),
    min((-b + sqrt(d)) / (2.0 * a), max_dist)
    );

    // if the ray did not hit the atmosphere, return a black color
    if (ray_length.x > ray_length.y) {
        return scene_color;
    }

    // prevent the mie glow from appearing if there's an object in front of the camera
    bool allow_mie = max_dist > ray_length.y;
    // make sure the ray is no longer than allowed
    ray_length.y = min(ray_length.y, max_dist);
    ray_length.x = max(ray_length.x, 0.0);
    // get the step size of the ray
    float step_size_primary = (ray_length.y - ray_length.x) / float(steps_primary);

    // next, set how far we are along the ray, so we can calculate the position of the sample
    // if the camera is outside the atmosphere, the ray should start at the edge of the atmosphere
    // if it's inside, it should start at the position of the camera
    // the min statement makes sure of that
    float ray_pos_i = ray_length.x + step_size_primary * 0.5;

    // these are the values we use to gather all the scattered light
    vec3 total_ray = vec3(0.0);// for rayleigh
    vec3 total_mie = vec3(0.0);// for mie

    // initialize the optical depth. This is used to calculate how much air was in the ray
    vec3 opt_i = vec3(0.0);

    // we define the density early, as this helps doing integration
    // usually we would do riemans summing, which is just the squares under the integral area
    // this is a bit innefficient, and we can make it better by also taking the extra triangle at the top of the square into account
    // the starting value is a bit inaccurate, but it should make it better overall
    vec3 prev_density = vec3(0.0);

    // also init the scale height, avoids some vec2's later on
    vec2 scale_height = vec2(height_ray, height_mie);

    // Calculate the Rayleigh and Mie phases.
    // This is the color that will be scattered for this ray
    // mu, mumu and gg are used quite a lot in the calculation, so to speed it up, precalculate them
    float mu = dot(dir, light_dir);
    float mumu = mu * mu;
    float gg = g * g;
    float phase_ray = 3.0 / (50.2654824574 /* (16 * pi) */) * (1.0 + mumu);
    float phase_mie = allow_mie ? 3.0 / (25.1327412287 /* (8 * pi) */) * ((1.0 - gg) * (mumu + 1.0)) / (pow(1.0 + gg - 2.0 * mu * g, 1.5) * (2.0 + gg)) : 0.0;

    // now we need to sample the 'primary' ray. this ray gathers the light that gets scattered onto it
    for (int i = 0; i < steps_primary; ++i) {

        // calculate where we are along this ray
        vec3 pos_i = start + dir * ray_pos_i;

        // and how high we are above the surface
        float height_i = length(pos_i) - planet_radius;

        // now calculate the density of the particles (both for rayleigh and mie)
        vec3 density = vec3(exp(-height_i / scale_height), 0.0);

        // and the absorption density. this is for ozone, which scales together with the rayleigh,
        // but absorbs the most at a specific height, so use the sech function for a nice curve falloff for this height
        // clamp it to avoid it going out of bounds. This prevents weird black spheres on the night side
        float denom = (height_absorption - height_i) / absorption_falloff;
        density.z = (1.0 / (denom * denom + 1.0)) * density.x;

        // multiply it by the step size here
        // we are going to use the density later on as well
        density *= step_size_primary;

        // Add these densities to the optical depth, so that we know how many particles are on this ray.
        // max here is needed to prevent opt_i from potentially becoming negative
        opt_i += max(density + (prev_density - density) * 0.5, 0.0);

        // and update the previous density
        prev_density = density;

        // Calculate the step size of the light ray.
        // again with a ray sphere intersect
        // a, b, c and d are already defined
        a = dot(light_dir, light_dir);
        b = 2.0 * dot(light_dir, pos_i);
        c = dot(pos_i, pos_i) - (atmo_radius * atmo_radius);
        d = (b * b) - 4.0 * a * c;

        // no early stopping, this one should always be inside the atmosphere
        // calculate the ray length
        float step_size_light = (-b + sqrt(d)) / (2.0 * a * float(steps_light));

        // and the position along this ray
        // this time we are sure the ray is in the atmosphere, so set it to 0
        float ray_pos_l = step_size_light * 0.5;

        // and the optical depth of this ray
        vec3 opt_l = vec3(0.0);

        // again, use the prev density for better integration
        vec3 prev_density_l = vec3(0.0);

        // now sample the light ray
        // this is similar to what we did before
        for (int l = 0; l < steps_light; ++l) {

            // calculate where we are along this ray
            vec3 pos_l = pos_i + light_dir * ray_pos_l;

            // the heigth of the position
            float height_l = length(pos_l) - planet_radius;

            // calculate the particle density, and add it
            // this is a bit verbose
            // first, set the density for ray and mie
            vec3 density_l = vec3(exp(-height_l / scale_height), 0.0);

            // then, the absorption
            float denom = (height_absorption - height_l) / absorption_falloff;
            density_l.z = (1.0 / (denom * denom + 1.0)) * density_l.x;

            // multiply the density by the step size
            density_l *= step_size_light;

            // and add it to the total optical depth
            opt_l += max(density_l + (prev_density_l - density_l) * 0.5, 0.0);

            // and update the previous density
            prev_density_l = density_l;

            // and increment where we are along the light ray.
            ray_pos_l += step_size_light;

        }

        // Now we need to calculate the attenuation
        // this is essentially how much light reaches the current sample point due to scattering
        vec3 attn = exp(-beta_ray * (opt_i.x + opt_l.x) - beta_mie * (opt_i.y + opt_l.y) - beta_absorption * (opt_i.z + opt_l.z));

        // accumulate the scattered light (how much will be scattered towards the camera)
        total_ray += density.x * attn;
        total_mie += density.y * attn;

        // and increment the position on this ray
        ray_pos_i += step_size_primary;

    }

    // calculate how much light can pass through the atmosphere
    vec3 opacity = exp(-(beta_mie * opt_i.y + beta_ray * opt_i.x + beta_absorption * opt_i.z));

    // calculate and return the final color
    return (
    phase_ray * beta_ray * total_ray// rayleigh color
    + phase_mie * beta_mie * total_mie// mie
    + opt_i.x * beta_ambient// and ambient
    ) * light_intensity + scene_color * opacity;// now make sure the background is rendered correctly
}

vec4 calculate_scattering_eye(
vec3 position, // the position of the scene geometry
mat4 view_matrix, // HACK we are using the camera orientation to position the planet correctly, since the given start position and direction are in camera space
vec4 scene_color, // the color of the scene
vec3 light_dir, // the direction of the light
vec3 light_intensity// how bright the light is, affects the brightness of the atmosphere
) {
    return vec4(calculate_scattering(
    vec3(0),
    normalize(position),
    length(position),
    view_matrix,
    scene_color.rgb,
    light_dir,
    light_intensity
    ), 1.0);
}
