import os
import re

files = [
    "./sanitizers/app/build.gradle",
    "./hello-jniCallback/app/build.gradle",
    "./native-codec/app/build.gradle",
    "./camera/basic/build.gradle",
    "./camera/texture-view/build.gradle",
    "./hello-vulkan/app/build.gradle",
    "./orderfile/app/build.gradle",
    "./sensor-graph/accelerometer/build.gradle",
    "./unit-test/app/build.gradle",
    "./exceptions/app/build.gradle",
    "./gles3jni/app/build.gradle",
    "./build.gradle",
    "./native-midi/app/build.gradle",
    "./hello-gl2/app/build.gradle",
    "./endless-tunnel/app/build.gradle",
    "./bitmap-plasma/app/build.gradle",
    "./native-audio/app/build.gradle",
    "./hello-jni/app/build.gradle",
    "./native-activity/app/build.gradle",
    "./teapots/textured-teapot/build.gradle",
    "./teapots/choreographer-30fps/build.gradle",
    "./teapots/classic-teapot/build.gradle",
    "./teapots/more-teapots/build.gradle",
    "./teapots/image-decoder/build.gradle"
]

patterns = [
    (r'^(\s*)namespace(?!\s*=)\s+(.*)', r'\1namespace = \2'),
    (r'^(\s*)applicationId(?!\s*=)\s+(.*)', r'\1applicationId = \2'),
    (r'^(\s*)versionCode(?!\s*=)\s+(.*)', r'\1versionCode = \2'),
    (r'^(\s*)versionName(?!\s*=)\s+(.*)', r'\1versionName = \2'),
    (r'^(\s*)path(?!\s*=)\s+(.*)', r'\1path = \2'),
    (r'^(\s*)viewBinding(?!\s*=)\s+(true|false)', r'\1viewBinding = \2'),
    (r'^(\s*)prefab(?!\s*=)\s+(true|false)', r'\1prefab = \2'),
    (r'^(\s*)shaders(?!\s*=)\s+(true|false)', r'\1shaders = \2'),
    (r'^(\s*)useLegacyPackaging(?!\s*=)\s+(true|false)', r'\1useLegacyPackaging = \2'),
    (r'^(\s*)testApplicationId(?!\s*=)\s+(.*)', r'\1testApplicationId = \2'),
    (r'^(\s*)useSupportLibrary(?!\s*=)\s+(true|false)', r'\1useSupportLibrary = \2'),
    (r'^(\s*)sourceCompatibility(?!\s*=)\s+(JavaVersion\.VERSION_\w+)', r'\1sourceCompatibility = \2'),
    (r'^(\s*)targetCompatibility(?!\s*=)\s+(JavaVersion\.VERSION_\w+)', r'\1targetCompatibility = \2'),
    (r'^(\s*)version(?!\s*=)\s+(.*)', r'\1version = \2'),
]

modified_files = []

for file_path in files:
    if not os.path.exists(file_path):
        continue
    
    with open(file_path, 'r') as f:
        lines = f.readlines()
    
    new_lines = []
    changed = False
    for line in lines:
        new_line = line
        # Only process lines that don't look like they are already using assignment
        # or are part of something else.
        # We also want to avoid matching things like 'minSdkVersion' if we are matching 'version'
        # but our patterns are specific enough.
        
        for pattern, replacement in patterns:
            if re.search(pattern, new_line):
                # Ensure we don't accidentally match something that has a prefix like 'test' if we don't want it.
                # Actually our patterns start with ^(\s*) so it should be fine.
                new_line = re.sub(pattern, replacement, new_line)
                changed = True
        new_lines.append(new_line)
    
    if changed:
        with open(file_path, 'w') as f:
            f.writelines(new_lines)
        modified_files.append(file_path)

print("Modified files:")
for f in modified_files:
    print(f)
