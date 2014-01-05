#include "PCA.h"

#include "Eigen/Geometry"
#include "Eigen/Eigenvalues"

Geom::PCA::PCA( QVector<Vector3> &pnts )
{
	mu = Vec3d(0.0, 0.0, 0.0);
	Eigen::Matrix3d C;

	// loop over the points to find the mean point
	// location
	foreach( Vec3d p, pnts)	mu += p;
	mu /= pnts.size();

	// loop over the points again to build the 
	// covariance matrix.  Note that we only have
	// to build terms for the upper triangular 
	// portion since the matrix is symmetric
	double cxx=0.0, cxy=0.0, cxz=0.0, cyy=0.0, cyz=0.0, czz=0.0;
	for( int i=0; i < (int)pnts.size(); i++ ){
		Vec3d &p = pnts[i];
		cxx += p.x()*p.x() - mu.x()*mu.x(); 
		cxy += p.x()*p.y() - mu.x()*mu.y(); 
		cxz += p.x()*p.z() - mu.x()*mu.z();
		cyy += p.y()*p.y() - mu.y()*mu.y();
		cyz += p.y()*p.z() - mu.y()*mu.z();
		czz += p.z()*p.z() - mu.z()*mu.z();
	}

	// now build the covariance matrix
	C(0,0) = cxx; C(0,1) = cxy; C(0,2) = cxz;
	C(1,0) = cxy; C(1,1) = cyy; C(1,2) = cyz;
	C(2,0) = cxz; C(2,1) = cyz; C(2,2) = czz;

	// extract the eigenvalues and eigenvectors from C
	Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(C);
	Eigen::Vector3d eigval = es.eigenvalues();
	Eigen::Matrix3d eigvec = es.eigenvectors();

	// find the right, up and forward vectors from the eigenvectors
	eigenVectors << Vector3( eigvec(0,0), eigvec(1,0), eigvec(2,0) )
		 << Vector3( eigvec(0,1), eigvec(1,1), eigvec(2,1) )
		 << Vector3( eigvec(0,2), eigvec(1,2), eigvec(2,2) );

	eigenValues = Vector3(eigval(0), eigval(1), eigval(2));
}
