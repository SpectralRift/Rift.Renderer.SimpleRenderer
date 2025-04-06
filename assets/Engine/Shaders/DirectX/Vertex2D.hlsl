struct R_VertexInput {
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    float4 color : COLOR;
};

struct R_VertexOutput {
    float4 position : POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    float4 color : COLOR;
};

uniform float4x4 ufProjMatrix;

R_VertexOutput main(R_VertexInput vin) {
    R_VertexOutput vout;

    vout.position = mul(float4(vin.position.xy, 1.0, 1.0), ufProjMatrix);
    vout.uv = float2(vin.uv.x, 1.0 - vin.uv.y);
    vout.normal = vin.normal;
    vout.color = vin.color.bgra;

    return vout;
}