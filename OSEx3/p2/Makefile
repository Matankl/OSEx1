CC = g++ 
CFLAGS = 
LDFLAGS = -lstdc++

all: MatrixD VectorD MatrixL VectorL

#matrix dequ 
MatrixD: MatrixD.o
	$(CC) $(CFLAGS) $(LDFLAGS) MatrixD.o -o MatrixD

MatrixD.o: matrixDequKosaraju.cpp
	$(CC) $(CFLAGS) -c $< -o $@

#vector dequ
VectorD: VectorD.o
	$(CC) $(CFLAGS) $(LDFLAGS) VectorD.o -o VectorD

VectorD.o: vectorDequKosaraju.cpp
	$(CC) $(CFLAGS) -c $< -o $@

#matrix list
MatrixL: MatrixL.o
	$(CC) $(CFLAGS) $(LDFLAGS) MatrixL.o -o MatrixL

MatrixL.o: matrixListKosaraju.cpp
	$(CC) $(CFLAGS) -c $< -o $@

#vector list
VectorL: VectorL.o
	$(CC) $(CFLAGS) $(LDFLAGS) VectorL.o -o VectorL

VectorL.o: vectorListKosaraju.cpp
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -f MatrixD.o  VectorD.o  MatrixL.o  VectorL.o  vectorListKosaraju matrixListKosaraju vectorDequKosaraju matrixDequKosaraju
