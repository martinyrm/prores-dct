import numpy as np
import cv2 
import sys

#DCT coefficients and shift values for 10-bit ProRes
W1 = 22725
W2 = 21407
W3 = 19265
W4 = 16384
W5 = 12873        
W6 = 8867
W7 = 4520

dc_shift = 1
extra_shift = 2 
row_shift = 13
col_shift = 18


def iDCTRow(row): #extrashift = 2
    row = row.flatten()
    a0 = (W4 * row[0])  + (1 << (row_shift + (extra_shift - 1)))
    a1 = a0 
    a2 = a0
    a3 = a0

    a0 += W2 * row[2]
    a1 += W6 * row[2]
    a2 -= W6 * row[2]
    a3 -= W2 * row[2]

    b0 = W1 * row[1]
    b0 += W3 * row[3]
    b1 = W3 * row[1]
    b1 += -W7 * row[3]
    b2 = W5 * row[1]
    b2 += -W1 * row[3]
    b3 = W7 * row[1]
    b3 += -W5 * row[3]

    a0 += W4 * row[4] + W6 * row[6]
    a1 += -W4 * row[4] - W2 * row[6]
    a2 += -W4 * row[4] + W2 * row[6]
    a3 += W4 * row[4] - W6 * row[6]

    b0 += W5 * row[5]
    b0 += W7 * row[7]

    b1 += -W1 * row[5]
    b1 += -W5 * row[7]

    b2 += W7 * row[5]
    b2 += W3 * row[7]

    b3 += W3 * row[5]
    b3 += -W1 * row[7]

    row[0] = (int) (a0 + b0) >> (row_shift + extra_shift)
    row[7] = (int) (a0 - b0) >> (row_shift + extra_shift)
    row[1] = (int) (a1 + b1) >> (row_shift + extra_shift)
    row[6] = (int) (a1 - b1) >> (row_shift + extra_shift)
    row[2] = (int) (a2 + b2) >> (row_shift + extra_shift)
    row[5] = (int) (a2 - b2) >> (row_shift + extra_shift)
    row[3] = (int) (a3 + b3) >> (row_shift + extra_shift)
    row[4] = (int) (a3 - b3) >> (row_shift + extra_shift)
    return np.transpose([row])

def idctSparseCol(col, index):
     col = col.flatten()
     a0 = W4 * (col[index] + ((1<<(col_shift-1))/W4))
     a1 = a0
     a2 = a0
     a3 = a0

     a0 += W2 * col[index + (8*2)]
     a1 += W6 * col[index + (8*2)]
     a2 += -W6 * col[index + (8*2)]
     a3 += -W2 * col[index + (8*2)]

     b0 = W1 * col[index + 8]
     b1 = W3 * col[index + 8]
     b2 = W5 * col[index + 8]
     b3 = W7 * col[index + 8]

     b0 += W3 * col[index + (8*3)]
     b1 += -W7 * col[index + (8*3)]
     b2 += -W1 * col[index + (8*3)]
     b3 += -W5 * col[index + (8*3)]

     a0 += W4 * col[index + (8*4)]
     a1 += -W4 * col[index + (8*4)]
     a2 += -W4 * col[index + (8*4)]
     a3 += W4 * col[index + (8*4)]

     b0 += W5 * col[index + (8*5)]
     b1 += -W1 * col[index + (8*5)]
     b2 += W7 * col[index + (8*5)]
     b3 += W3 * col[index + (8*5)]

     a0 += W6 * col[index + (8*6)]
     a1 += -W2 * col[index + (8*6)]
     a2 += W2 * col[index + (8*6)]
     a3 += -W6 * col[index + (8*6)]
     

     b0 += W7 * col[index + (8*7)]
     b1 += -W5 * col[index + (8*7)]
     b2 += W3 * col[index + (8*7)]
     b3 += -W1 * col[index + (8*7)]

     col[index] = ((int) (a0 + b0) >> col_shift)
     col[index + 8] = ((int) (a1 + b1) >> col_shift)
     col[index + 16] = ((int) (a2 + b2) >> col_shift)
     col[index + 24] = ((int) (a3 + b3) >> col_shift)
     col[index + 32] = ((int) (a3 - b3) >> col_shift)
     col[index + 40] = ((int) (a2 - b2) >> col_shift)
     col[index + 48] = ((int) (a1 - b1) >> col_shift)
     col[index + 56] = ((int) (a0 - b0) >> col_shift)
     return col

orig_dct_block = dct_block
all = np.fromfile("/Users/martinr/all_blocks_prepost.dat", dtype=np.int16)
success = 0
fail = 0
for i in range(0, 2500):
    #print("Block number:" + str(i))
    block = all[i*128:((i+1) * 128) - 64].reshape(64,1)
    original_block = block
    ref = all[(i*128) + 64:((i+1) * 128)].reshape(64,1)

    block = np.int64(block)

    for i in range(0,8,1):
        nrow = block[i*8:(i+1)*8]
        idct_row = iDCTRow(nrow)
        block[i*8:((i+1)*8)] = idct_row

    for i in range(0,8,1):
        block[i] += 8192
        block = idctSparseCol(block, i)

    if (block.reshape(8,8) == ref.reshape(8,8)).all():
        success += 1
        #print(original_block.reshape(8,8))
        #print(qm.reshape(8,8))
    else:
         #print("Block failed to match FFmpeg output")
         #print("These are the DCT coefficients decoded by FFmpeg:")
         #print(original_block.reshape(8,8))
         #print("This is the result of FFmpeg's ProRes IDCT applied on the above")
         #print(ref.reshape(8,8))
         #print("This is our attempt of applying the same IDCT")
         #print(block.reshape(8,8))
         #print("Quantization matrix used:")
         #print(qm.reshape(8,8))
         fail += 1
print("Failed: " + str(fail))
print("Succeeded: " + str(success))

