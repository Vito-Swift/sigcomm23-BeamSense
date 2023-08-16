# sigcomm23-BeamSense
Simulation, multi-path estimation, and CBR parsing code of SIGCOMM2023 BeamSense CBR-Sensing

## Multi-path Estimation
### Installation
```
git clone https://github.com/Vito-Swift/sigcomm23-BeamSense.git
cd sigcomm23-BeamSense
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j4
```

### Run Example
Back to `sigcomm23-BeamSense` directory

`./build/example dataset/V.mat dataset/S.mat dataset/UH.mat 5 234 3 3`

### Data Format
We suggest to use `numpy` to generate each matrix.

For example:
```
num_tx = 3
num_sts = 3
num_sc = 234
V = numpy.zeros((num_sc, num_tx, num_sts), dtype=complex) # here fill correct matrix 
V.tofile('V.mat')
```