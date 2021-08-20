import sys,os
sys.path.append(os.path.join(os.environ['ATS_SRC_DIR'],'tools','meshing_ats'))

import meshing_ats
import numpy as np
from matplotlib import pyplot as plt



# Defining the geometry and meshing for the Yakou catchement

# Step 1 - The surface with 20° slope
# Length of the catchment = 1000 m, slope = 20°, dx_surface = 10 m, dz_surface =  2 m? or defined later in step 2?


# 1 km long hillslope, 20% slope
x = np.linspace(0,1000,101)
z = 200 - 0.2*x 
# Gradient = 200/1000 = 0.2

m2 = meshing_ats.Mesh2D.from_Transect(x,z) # Creates a recatangular mesh with x + z nodes, x - 1 number of cells

### Step 2 - The vertical cross-section [along a single column]

### To define the vertical properties of the Yakou catchment (Assumptions)


layer_types_yakou = [] # Assuming 'constant' for each layers, i.e. dz within the layer is constant -
                       # Q - What other options are available?

layer_data_yakou = [] # this data depends upon the layer type, but
                # for constant is the thickness of the layer

layer_ncells_yakou = [] # number of cells (in the vertical) in the layer.
                        # The dz of each cell is the layer thickness / number of cells. - Why is it required?

layer_mat_ids_yakou = [] # The material ID.  This may be either a constant int (for
                   # unform layering) or an array of size [ncells_vertical x ncells_horizontal] in the layer 
                  # where each entry corresponds to the material ID of that cell.

layer_depth_yakou = []  # used later to get the mat ids right, just for bookkeeping

# We are considering 3 layers - Organic layer, Mineral layer, Bedrock
# Requirement to find the depth of the individual layers - 
# Research articles! (Conduct ERT/IP experiments and geological experiments)
### Assuming the lower depth of each layers:

depth_org_layer = 0.60 # m
depth_mineral_layer = 30 # m
depth_bedrock = 40 # m
        
# Starting length of each vertical cell, dz = 0.01
# We will also telescope the mesh, starting at 0.01 m grid cell and growing it larger in each layer.
# The dz values vary as such, For the organic layer - 1.2, mineral layer - 1.4, Bedrock - 1.5 
# Can change the values later and check if the results are affected

dz = 0.01
i = 0
current_depth = 0

while current_depth <= depth_bedrock:
    if (current_depth >= 0) & (current_depth <= depth_org_layer):     # organic
        dz *= 1.2 
    elif (current_depth > depth_org_layer) & (current_depth <= depth_mineral_layer): # mineral
        dz *= 1.4
    elif (current_depth > depth_mineral_layer) & (current_depth < depth_bedrock): # bedrock
        dz *= 1.5
    layer_types_yakou.append("constant")
    layer_data_yakou.append(dz) # Thickness of all the dz's will be added in the numpy array 
    layer_ncells_yakou.append(1) # Why append only 1? - Is'nt it the dz value?
    current_depth += dz
    layer_depth_yakou.append(current_depth)
    i += 1

# Removing the last element from the lists as it exceeds 40 m
layer_types_yakou.pop()
layer_data_yakou.pop()
#layer_ncells_yakou.pop()
layer_depth_yakou.pop()

# now add in a bunch of cells to reach 40 m
num_of_layers=len(layer_data_yakou)
layer_types_yakou.append('constant')
layer_data_yakou.append(depth_bedrock - sum(layer_data_yakou))  # note sum(layer_data) == the total mesh thickness at this point
#layer_ncells_yakou.append(int(np.floor(layer_data_yakou[-2]/dz)))
#layer_ncells_yakou.append(1)
layer_depth_yakou.append(depth_bedrock)

# Note that the boundaries between the layers are not clearly defined


#### Step 3:Allocate 2D matrix with unique IDs based on the layer
#### For organic layer - 1001, mineral layer = 1002, bedrock = 1003 

##### Note that the maximum depth of the layers is considered for allocating the IDs 

mat_ids = np.zeros((m2.num_cells(), len(layer_depth_yakou)), 'i') # Creating a numpy array with zeros (int value)
for i in range(m2.num_cells()):
    for j in range(len(layer_depth_yakou)): # len(layer_depth_yakou) - Number of cells in the vertical depth
        if (layer_depth_yakou[j] < depth_org_layer):
            mat_ids[i,j] = 1001 # Organic layer
        elif ((layer_depth_yakou[j] >= depth_org_layer) & (layer_depth_yakou[j] < depth_mineral_layer)):
            mat_ids[i,j] = 1002 # Mineral layer
        else:
            mat_ids[i,j] = 1003 # Bedrock

# filling out layer_mat_ids
layer_mat_ids_yakou = []
for j in range(len(layer_depth_yakou)):
    layer_mat_ids_yakou.append(mat_ids[:,j])
# Q - Why is the next 2 lines of code written?
for j in range(len(layer_depth_yakou),sum(layer_ncells_yakou)):
    layer_mat_ids_yakou.append(101*np.ones((100,),'i'))
    

# make the mesh, save it as an exodus file
m3 = meshing_ats.Mesh3D.extruded_Mesh2D(m2, layer_types_yakou,layer_data_yakou, layer_ncells_yakou, layer_mat_ids_yakou)
m3.write_exodus("Yakou_model_trial.exo")