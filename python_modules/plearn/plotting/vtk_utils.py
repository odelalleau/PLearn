# Module containing utility functions to generate .vtk files


def create_unstructured_vector_file(vtkfilename, vec_origin, vec_end, vecname="vec", datatitle="unstructured vector"):
    """Creates a VTK unstructured grid populated with n 3d vectors going
    from vec_origin to vec_end (both must be of dimension n times 3).
    The vec_origin will serve as the unstructured grid points,
    while the the VTK VECTORS point_data (named vecname) will be populated
    by the difference between vec_end and vec_origin."""

    l = len(vec_origin)

    f = file(vtkfilename,"w")
    f.write("# vtk DataFile Version 3.1\n")
    f.write(datatitle+"\n")

    f.write("ASCII\n")
    f.write("DATASET UNSTRUCTURED_GRID\n")
    f.write("POINTS %d FLOAT\n"%l)
    
    for x,y,z in vec_origin:
        f.write("%f %f %f\n"%(x,y,z))

    f.write("\n")
    f.write("POINT_DATA %d \n"%l)
    f.write("VECTORS "+vecname+" FLOAT\n")
    for i,(x2,y2,z2) in enumerate(vec_end):
        x1,y1,z1 = vec_origin[i]
        dx = float(x2-x1)
        dy = float(y2-y1)
        dz = float(z2-z1)
        f.write("%f %f %f\n"%(dx,dy,dz))

    f.close()


def create_vector_file_from_2d_weighted_samples(vtkfilename, x_y_weight, vecname="weighted_samples"):
    vec_origin = []
    for x,y,w in x_y_weight:
        vec_origin.append((x,y,0))
    vec_end = x_y_weight
    create_unstructured_vector_file(vtkfilename, vec_origin, vec_end, vecname=vecname, datatitle=vecname)


def create_2d_grid_values_file(vtkfilename, x0, y0, dx, dy, zmat, valuename="Scalars", datatitle="2d grid values"):
    """ zmat -- A 2D array for the x and y points with x varying fastest (column indexes)
        and y next (row indexes). """

    nx = len(zmat[0])
    ny = len(zmat)

    l = nx*ny

    f = file(vtkfilename,"w")
    f.write("# vtk DataFile Version 3.1\n")
    f.write(datatitle+"\n")

    f.write("ASCII\n")
    f.write("DATASET STRUCTURED_POINTS\n")
    f.write("DIMENSIONS %d %d %d\n"%(nx,ny,1))
    f.write("ORIGIN %f %f %f\n"%(x0,y0,0))
    f.write("SPACING %f %f %f\n"%(dx,dy,1))

    f.write("\n")
    f.write("POINT_DATA %d \n"%l)

    f.write("SCALARS "+valuename+" FLOAT\n")
    f.write("LOOKUP_TABLE default\n")

    for row in zmat:
        for elem in row:
            f.write("%f\n"%elem)

    f.close()

def create_2d_grid_values_file_from_regular_x_y_z(vtkfilename, x, y, z):

    if callable(z):
        zmat = [ [z(xv, yv) for yv in y] for xv in x ]
    else:
        zmat = z
        
    create_2d_grid_values_file(vtkfilename, x[0], y[0],
                               x[1]-x[0], y[1]-y[0], zmat)    

def create_2d_grid_values_file_from_regular_xymagnitude(vtkfilename, regular_xymagnitude):
    x,y,gridvalues = xymagnitude_to_x_y_grid(regular_xymagnitude)
    create_2d_grid_values_file_from_regular_x_y_z(x, y, gridvalues)

