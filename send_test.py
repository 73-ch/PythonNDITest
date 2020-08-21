import cv2
import numpy as np
from pythonndi import NDISender

if __name__ == "__main__":
    sender = NDISender()
    image = cv2.imread("./test.jpg")
    image2 = cv2.imread("./test2.jpg")
    b, g, r = cv2.split(image2)
    alpha = np.ones(r.shape, dtype=r.dtype) * 255
    image_rgba = cv2.merge((b, g, r, alpha))

    i = 0

    print(image2.shape)
    while True:
        i = (i + 1) % 30
        print(i)
        mono = np.ones(r.shape, dtype=r.dtype) * int(255 / 30 * i)
        frame = cv2.merge((mono, mono, mono, alpha))
        if i > 15:
            sender.send(image)
        else:
            sender.send(image2)

