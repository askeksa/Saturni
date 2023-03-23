
## Install on Ubuntu 22.04

```
sudo apt-get install libglfw3
sudo apt-get install libglfw3-dev
sudo apt-get install libglew-dev
sudo apt-get install libluajit-5.1-dev
sudo apt-get install portaudio19-dev 
```

## Getting Started

Minimal lua file

```lua
title = "Minimal Test"

music = "test.wav"
shader = "testshader.glsl"

width = 640
height = 480

i = 0
function frame()
  if i % 30 == 0 then
    print("frame:"..i)
  end
  i = i + 1
end
```

run using: ` ./build/saturni example.lua`