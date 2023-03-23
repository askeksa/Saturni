title = "Minimal Test"

music = "test.wav"
shader = "helloworld.glsl"

width = 640
height = 480

i = 0
function frame()
  if i % 30 == 0 then
    print("frame:"..i)
  end
  i = i + 1
end
