
import numpy as np 
import matplotlib.pyplot as plt 
import cv2

def f(x: np.ndarray, y: np.ndarray, xc: float, yc: float, sx: float, sy: float) -> np.ndarray:
    return np.exp( -sx*(x-xc)**2 - sy*(y-yc)**2  )

def fx(x: np.ndarray, y: np.ndarray, xc: float, yc: float, sx: float, sy: float) -> np.ndarray:
    return -2*sx*(x-xc)*f(x, y, xc, yc, sx, sy)

def fy(x: np.ndarray, y: np.ndarray, xc: float, yc: float, sx: float, sy: float) -> np.ndarray:
    return -2*sy*(y-yc)*f(x, y, xc, yc, sx, sy)

def save_normal_mapU16(fx: np.ndarray, fy: np.ndarray, fz: np.ndarray,   image_name: str):
    PVAL = (1<<16)-1    

    r = ((1.0+fx)*0.5*PVAL).astype(np.uint16)
    g = ((1.0+fy)*0.5*PVAL).astype(np.uint16)
    b = ((1.0+fz)*0.5*PVAL).astype(np.uint16)

    image = cv2.merge( (b, g, r) )
    cv2.imwrite(image_name, image)

def save_r32_file(z: np.ndarray,   image_name: str):
    rows, cols = z.shape
    with open(image_name, "wb") as ofile:
        header = np.array([rows, cols], dtype=np.single)    
        header.tofile(ofile)
        z.astype(np.single).tofile(ofile)



if __name__ == '__main__':
    NX = 2000 
    NY = 1000 

    x = np.linspace(-4, 4, NX)
    y = np.linspace(-2, 2, NY)
    x, y = np.meshgrid(x,y)

    
    zz = np.zeros_like(x)
    zx = np.zeros_like(x)
    zy = np.zeros_like(x)

    for params in [(-0.5,  0.5, 21, 31), 
                   ( 0.0,  0.2, 12, 9), 
                   ( 0.0,  0.0, 32, 7),
                   ( 0.3, -0.5, 7, 71),
                   ( 0.3,  0.5, 17, 71)]:
        zz += f(x,y, *params) 
        zx += fx(x,y, *params) 
        zy += fy(x,y, *params) 
    
    nx = -zx
    ny =  zy  # since y axis is inverted 
    nz =  np.ones_like(nx)

    norm = np.sqrt(nx**2+ny**2+nz**2)
    nx/=norm
    ny/=norm
    nz/=norm

    dx, dy = x[0,1]-x[0,0], y[1,0]-y[0,0]
    print(f"DX={dx} DY={dy}")
    save_normal_mapU16(fx=nx, fy=ny, fz=nz, image_name="TEST00.tiff")    
    save_r32_file(zz, "TEST00.r32")
