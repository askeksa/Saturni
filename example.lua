title = "Minimal Test"

framerate = 60
music = "test.wav"
shader = "helloworld.glsl"

width = 640
height = 480

-- a global getter for `u_time` which is what we have as a uniform
-- in our shader glsl to get the shader playback time in sec 
setmetatable(_G, {__index = function (table, key)
   if key == "u_time" then return (i / 60) end
   if key == "u_resolution" then return {x = width, y = height} end
  end
})

i = 0
function frame()
  if i % 30 == 0 then
    print("frame:"..i)
  end
  i = i + 1
end
