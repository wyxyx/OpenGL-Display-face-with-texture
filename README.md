# OpenGL-Display-face-with-texture

#### Set up  
•	 Draw a 600x600 window.  
•	 Use Perspective projection, set the field of view angle to be 45 degree, near plane to be 0.1 and far plane to be 100.  
•	 Camera movements: Use ← and → keys move the camera along the blue circle parallel to the equator. ↑ and ↓ keys rotate the camera along the red circle orthogonal to the equator. Point the camera always to the origin. Choose a good "up" direction.  
•	 Key 'r' resets the program to its startup state. (displays control points on x-y plane, clears any rotations, etc.)  
#### Display face geometry as spline surface  
•	 Create a coarse quad mesh (only the front face is needed)  
•	 Read the quad mesh as an obj file into your openGL program and display it. The user should be able to show/hide the mesh using the key f.  
#### Add a texture  
•	 Take a photo of your face.  
•	 Map the photo onto the mesh and display.  
#### Render a smooth surface  
•	 render the face smoothly (with high refinement level) using the tensor of either of the representations(state method in Readme.txt)
•	 uv-map the texture correctly onto the curved surface. This must be clearly visible.  
#### Animate the face  
•	 enable picking to animate the face; show a smile and a frown.  
