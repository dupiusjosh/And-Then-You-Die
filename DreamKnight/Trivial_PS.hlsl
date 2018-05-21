struct Light
{
    float4 position; //xyz = pos, w = type
    float4 direction; //xyz = dir, w = distance 
    float4 color; //xyzw = color
};

#define LIGHT_MAX 32

cbuffer Lights : register(b0)
{
    Light lights[LIGHT_MAX];
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 orig_pos : POSITION;
    float4 color : COLOR;
    float4 norm : NORMAL;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 lightColor;

    for (uint i = 0; i < LIGHT_MAX; ++i)
    {
        if (lights[i].position.w == 0.0f)
            break;
        switch (lights[i].position.w)
        {
            case 1.0f:{
                    lightColor += lights[i].color;
                    break;
                }
            case 3.0f:{
                    float ratio = 0.0;
                    float4 dir;
                    dir = lights[i].position;
                    dir[3] = 0;
                    dir = normalize(dir - input.orig_pos);
                    ratio = dot(dir.xyz, input.norm.xyz);
                    ratio = saturate(ratio);
                    ratio *= (1.0 - saturate(length(lights[i].position - input.orig_pos) / lights[i].direction[3]));
                    lightColor += saturate((lights[i].color * ratio));
                    break;
                }
        }
    }
    
    lightColor.a = 1;
    input.color.w = 1.0f;
    return float4(1.0, 1.0, 1.0, 1.0);
    //float4(1.0, 1.0, 1.0, 1.0); //* lightColor;
}