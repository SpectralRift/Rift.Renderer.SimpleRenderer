struct VertexIn {
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    float4 color : COLOR;
};

struct VertexOut {
    float4 position : POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    float4 color : COLOR;
};

uniform float4x4 ufProjMatrix;
uniform float4x4 ufViewMatrix;
uniform float4x4 ufModelMatrix;

VertexOut main(VertexIn vin) {
    VertexOut vout;

    vout.position = mul(mul(mul(float4(vin.position, 1.0), ufModelMatrix), ufViewMatrix), ufProjMatrix);
    vout.uv = float2(vin.uv.x, 1.0 - vin.uv.y);
    vout.normal = vin.normal;
    vout.color = vin.color.bgra;

    return vout;
}