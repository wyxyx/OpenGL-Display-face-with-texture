The link of my running program: https://recordit.co/Du06hEFEGk

1.Instructions to run the program:

Press key ←: move the camera to left
Press key →: move the camera to right
Press key ↑: move the camera to up
Press key ↓: move the camera to down
Press key r: reset the program
Press key f: show/hide the mesh
Press key t: show/hide the texture on mesh
Press key b: show/hide the mesh smoothly
Press key s: show/hide the texture on the smooth mesh
Pick the point near the lips: animate the face to show a smile
Pick the point near the eyebrows: animate the face to show a frown 

2.The method of rendering a smooth surface
In task 3, I use the formula of a bicubic Bezier surface:
c00 = (16*s11+4*(s21+s12+s01+s10)+(s22+s02+s00+s20))/36
c10 = (8*s11+2*(s10+s12)+4*s21+(s22+s20))/18
c11 = (4*s11+2*(s21+s12)+s22)/9