R""(
#version 400 core

out vec4 FragmentColor;

in vec3 VertexColor;

void main()
{
  FragmentColor = vec4(VertexColor, 1.f);
}
)"";
