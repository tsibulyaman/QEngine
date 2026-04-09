#version 330 core
uniform vec3 uIdColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(uIdColor, 1.0);
}
