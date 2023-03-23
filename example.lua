title = "Minimal Test"

framerate = 60
music = "test.wav"
shader = "helloworld.glsl"

width = 640
height = 480

-- a global getter for `u_time` which is what we have as a uniform
-- in our shader glsl to get the shader playback time in sec 
setmetatable(_G, {__index = function (table, key)
   if key == "u_time" then return ElapsedTime end
   if key == "u_resolution" then return {x = width, y = height} end
  end
})

Frames = 0 -- number of times frame has been called
ElapsedTime = 0 -- elapsed time in sec, note: can go backwards!

function frame(t)
  if Frames % 30 == 0 then
    print("time(s):"..t)
  end
  ElapsedTime = t
  Frames = Frames + 1
end
