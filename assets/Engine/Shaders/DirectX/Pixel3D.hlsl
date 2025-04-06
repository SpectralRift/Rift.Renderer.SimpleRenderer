struct R_VertexInput {
    float4 position : POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    float4 color : COLOR;
};

sampler2D s_Texture : register(s0);

float4 main(R_VertexInput v) : COLOR {
    return tex2D(s_Texture, v.uv) * v.color;
}