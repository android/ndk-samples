attribute float vPosition;
attribute float vSensorValue;

void main() {
    gl_Position = vec4(vPosition, vSensorValue/9.81, 0, 1);
}
