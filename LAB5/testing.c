// ======================= TEST 1 =======================
    diffSpeed = 0; // disable steering
    desSpeed = tilt / 256.0;

    if (fabs(desSpeed) < 0.1) desSpeed = 0;

    if (TachR_avg > 0)
    {
        real_pwmR = 15000.0 / TachR_avg;
    }
    else
    {
        real_pwmR = 0;
    }
    
    if (TachL_avg > 0)
    {
        real_pwmL = 15000.0 / TachL_avg;
    }
    else
    {
        real_pwmL = 0;
    }

    printf("desSpeed: %.2f, L: %.2f, R: %.2f\r\n", desSpeed, real_pwmL, real_pwmR);

// ======================= TEST 2 =======================
    desSpeed = 0; // disable pitch control

    // heading control as usual
    last_Heading_Error = Heading_Error;
    Heading_Error = heading_desired - Heading;
    if (Heading_Error > 1800) Heading_Error -= 3600;
    if (Heading_Error < -1800) Heading_Error += 3600;

    diffSpeed = kp_heading * Heading_Error + kd_heading * (Heading_Error - last_Heading_Error);
    if (diffSpeed > 0.5) diffSpeed = 0.5;
    if (diffSpeed < -0.5) diffSpeed = -0.5;

    if (TachR_avg > 0)
    {
        real_pwmR = 15000.0 / TachR_avg;
    }
    else
    {
        real_pwmR = 0;
    }
    
    if (TachL_avg > 0)
    {
        real_pwmL = 15000.0 / TachL_avg;
    }
    else
    {
        real_pwmL = 0;
    }

    printf("Desired Heading: %d, Enc Heading: %.2f, diffSpeed: %.2f, L: %.2f, R: %.2f\r\n",
           heading_desired, heading_rm, diffSpeed, real_pwmL, real_pwmR);

// ======================= TEST 3 =======================
    desSpeed = 0;
    diffSpeed = 0;
    // Just let the robot sit and measure heading drift
    printf("Compass Heading: %d, Encoder Heading: %.2f\r\n", Heading, heading_rm);
