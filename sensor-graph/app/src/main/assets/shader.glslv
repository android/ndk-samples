attribute float vPositionX;
attribute float vSensorValue;

void main() {
    gl_Position = vec4(vPositionX, vSensorValue, 0, 1);
}
