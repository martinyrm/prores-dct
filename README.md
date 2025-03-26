# GLSL Inverse Discrete Cosine Transform (for 10-bit ProRes)

This repository contains a port of FFmpeg's ProRes iDCT to GLSL, including OpenGL host code, and verification tests. This is a naïve translation from [](libavcodec/simple_idct_template.c), where no attempt has been made to optimise for execution on GPU and the shader operates on 8x8 blocks at a time using the separable fast iDCT.

## Verification

Reference transform data was obtained by running FFmpeg on a ProRes video sample. (libavcodec/proresdsp.c) was modified to write out all pre and post-transform DCT blocks to a file. 300 blocks from a real video sample (out2.mov) are used to verify the GLSL and Python implementations in this repo.

## GLSL implementation

The transform is implemented as a compute shader, with an int16[64] wide SSBO used for passing the transform blocks in and out of the GPU. No attempt was made to parallelise the computation, each DCT block is passed to a new shader invocation in turn. 


## Python implementation
Used as a testing platform - included at (python/) for completeness only 



