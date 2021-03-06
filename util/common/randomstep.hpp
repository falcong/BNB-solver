#ifndef _RANDOM_STEP_
#define _RANDOM_STEP_
#include "vec.hpp"
#include "util.hpp"
#include "bnblog.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>



template <typename FT> class RandomStep{
	public:
		virtual void next(int dim, FT* base, FT* bounds, FT* res) = 0;
		
		void next(int dim, FT* base, FT r, FT* res){
			FT* bounds = new FT[dim];
			GetBounds(dim,r,bounds);
			next(dim,base,bounds,res);
			delete[] bounds;
		};
		
		virtual void GetBounds(int dim,FT r, FT * bounds) = 0;
		virtual int GetBoundsSize(int dim) = 0; //size in bytes
};

template <typename FT> class HyperPlane{
	public:
		HyperPlane(int dim, FT *g){
			mDim = dim;
			mG = (FT*)malloc(mDim*sizeof(FT));
			mC = (FT*)malloc((mDim+1)*sizeof(FT));
			mA = (FT*)malloc(mDim*mDim*sizeof(FT));
			
			mGnorm = VecUtils::vecNormTwo<FT>(dim, g);
			VecUtils::vecMult<FT>(dim, g,1.0/mGnorm,mG);
			
			for(int i = 0; i < mDim; ++i){
				mC[i] = 1.0;
				for(int j = 0; j < i; ++j){
					mC[i] -= mG[j]*mG[j];
				};
				mC[i] = sqrt(mC[i]);
			};


			for(int i = 0; i < mDim-1; ++i){
				for(int j = 0; j < i; ++j){
					mA[i*mDim+j] = - mG[j]*mG[j+1]/mC[j]/mC[j+1];
				};
				
				mA[i*mDim+i] = mC[i+1]/mC[i];
				
				for(int j = i+1; j < mDim-1; ++j){
					mA[i*mDim+j] = 0.0;
				};
				
				mA[i*mDim+mDim-1] = mG[i];
			};
			
			for(int j = 0; j < mDim-2; ++j){
				mA[(mDim-1)*mDim+j] = - mG[j]*mC[mDim-1]/mC[j]/mC[j+1];				
			};
			
			mA[(mDim-1)*mDim+mDim-2] = -mG[mDim-2]/mC[mDim-2];
			mA[(mDim-1)*mDim+mDim-1] = mC[mDim-1];
		};
		
		~HyperPlane(){
			if (mG)
				free(mG);
			if (mA)
				free(mA);
			if (mC)
				free(mC);
		};
		
		void convertCoords(const FT* x, FT* y){
			for(int i = 0; i < mDim; ++i){
				y[i] = VecUtils::vecScalarMult<FT>(mDim-1, mA+mDim*i, x);
			}
		}
		
		void getMatrix(FT* A) {
			VecUtils::vecCopy<FT>(mDim*mDim, mA, A);
		};
	
	
	private:
		FT* mG;
		FT* mC;
		FT mGnorm;
		FT* mA;
		int mDim;
};
	



template <typename FT> class HyperPlaneUniformSphere : public RandomStep<FT>{
	public:
		HyperPlaneUniformSphere (int dim, FT* grad){
			mGrad = (FT*)malloc(dim*sizeof(FT));
			mTempRes = (FT*)malloc(dim*sizeof(FT));
			VecUtils::vecCopy<FT>(dim, grad, mGrad);
			mHP = new HyperPlane<FT>(dim, mGrad);
		};
		
		~HyperPlaneUniformSphere (){
			if (mGrad)
				free (mGrad);
			if (mTempRes)
				free (mTempRes);
			if (mHP)
				delete mHP;
		};
		
		void next(int dim, FT* base, FT* bounds, FT* res){
			mLog << "HPUS: Hyper plane uniform sphere initiated.\n";
			//~ mLog.write();
			FT norm;
			norm = 0.;
			for(int j = 0; j < dim-1; j ++) {
				mTempRes[j] = Gauss();
				norm += mTempRes[j] * mTempRes[j];
			};
			mLog << "HPUS: Gauss initial random seed in hyperplane: ";
			for(int j = 0; j < dim-1; j ++) {
				mLog << mTempRes[j] << " ";
			}
			mLog << "\n";
			//~ mLog.write();
			FT radius = *bounds;
			mLog << "HPUS: Radius = " << radius << "\n";
			//~ mLog.write();
			FT a;
			a = radius / sqrt(norm);
			for(int j = 0; j < dim; j ++) {
				mTempRes[j] *= a;
			};

			mLog << "HPUS: Random step in hyperplane magnified to radius: ";
			for(int j = 0; j < dim-1; j ++) {
				mLog << mTempRes[j] << " ";
			}
			mLog << "\n";
			//~ mLog.write();
			
			mHP->convertCoords(mTempRes,res);

			mLog << "HPUS: Random step converted to n-dimension space: ";
			for(int j = 0; j < dim; j ++) {
				mLog << res[j] << " ";
			}
			mLog << "\n";
			//~ mLog.write();
			
			if(base) {
				for(int j = 0; j < dim; j ++) {
					res[j] += base[j];
				};
			};
			
			mLog << "HPUS: Random step added to base: ";
			for(int j = 0; j < dim; j ++) {
				mLog << res[j] << " ";
			}
			mLog << "\n";
			mLog.write();

		};
		
		void GetBounds(int dim,FT r, FT * bounds){
			*bounds = r;
		};
		
		int GetBoundsSize(int dim){
			return sizeof(FT);
		};

	
	private:
		FT* mGrad;
		FT* mTempRes;
		HyperPlane<FT>* mHP;
		BNBLog <BNB_NO_LOGGING> mLog;
		FT Gauss() {
			int SeqLen = 30;
			FT v,u;
			v = 0.;
			for(int i = 0; i < SeqLen; i ++) {
				u = ((FT)rand())/ ((FT)RAND_MAX);
				v += u;
			}	
			return v - (0.5 * ((FT)SeqLen));
		};
	
};

template <typename FT> class HyperPlaneUniformBall : public RandomStep<FT>{
	public:
		HyperPlaneUniformBall (int dim, FT* grad){
			mGrad = (FT*)malloc(dim*sizeof(FT));
			mTempRes = (FT*)malloc(dim*sizeof(FT));
			VecUtils::vecCopy<FT>(dim, grad, mGrad);
			mHP = new HyperPlane<FT>(dim, mGrad);
		};
		
		~HyperPlaneUniformBall (){
			if (mGrad)
				free (mGrad);
			if (mTempRes)
				free (mTempRes);
			if (mHP)
				delete mHP;
		};
		
		void next(int dim, FT* base, FT* bounds, FT* res){
			FT norm;
			norm = 0.;
			for(int j = 0; j < dim-1; j ++) {
				mTempRes[j] = Gauss();
				norm += mTempRes[j] * mTempRes[j];
			};
			FT radius = *bounds;
			FT a;
			a = radius / sqrt(norm);
			for(int j = 0; j < dim; j ++) {
				mTempRes[j] *= a;
			};
			
			mHP->convertCoords(mTempRes,res);
			
			if(base) {
				for(int j = 0; j < dim; j ++) {
					res[j] += base[j];
				};
			};
		};
		
		void GetBounds(int dim,FT r, FT * bounds){
			bounds = &r;
		};
		
		int GetBoundsSize(int dim){
			return sizeof(FT);
		};

	
	private:
		FT* mGrad;
		FT* mTempRes;
		HyperPlane<FT>* mHP;
		FT Gauss() {
			int SeqLen = 30;
			FT v,u;
			v = 0.;
			for(int i = 0; i < SeqLen; i ++) {
				u = ((FT)rand())/ ((FT)RAND_MAX);
				v += u;
			}	
			return v - (0.5 * ((FT)SeqLen));
		};
	
};

template <typename FT> class MultiNormal : public RandomStep<FT>{
	public:
		void next(int dim, FT* base, FT* bounds, FT* res){
			FT* Z = new FT[dim];
			for(int j = 0; j < dim; ++j){
				Z[j] = Gauss();
			};
			
			for(int i = 0; i < dim; ++i){
				res[i] = base[i];
				for(int j = 0; j < dim; ++j){
					res[i] += bounds[i*dim+j] * Z[j];
				};
			}; 
			delete[] Z;
		};
		
		int GetBoundsSize(int dim){
			return dim*dim * sizeof(FT);
		};
		
		void GetBounds(int dim, FT r, FT * bounds){
			for(int i = 0; i < dim; ++i){
				for(int j = 0; j < dim; ++j){
					if (i == j){
						bounds[i*dim+j] = sqrt(r);
					} else {
						bounds[i*dim+j] = 0.0;
					};
				};
			};
		};
		
	private:
		FT Gauss() {
			int SeqLen = 30;
			FT v,u;
			v = 0.;
			for(int i = 0; i < SeqLen; i ++) {
				u = ((FT)rand())/ ((FT)RAND_MAX);
				v += u;
			}	
			return v - (0.5 * ((FT)SeqLen));
		};
};

template <typename FT> class UniformCubic : public RandomStep<FT>{
	public:
		void next(int dim, FT* base, FT* bounds, FT* res){
			FT u;
			for(int j = 0; j < dim; ++j){
				u = ((FT)rand())/ ((FT)RAND_MAX);
				res[j] = base[j] + bounds[j] * (2.0*u - 1.0);
			};
		};
		
		void GetBounds(int dim,FT r, FT * bounds){
			for(int j = 0; j < dim; ++j){
				bounds[j] = r;
			};
		};
		
		int GetBoundsSize(int dim){
			return dim * sizeof(FT);
		};
};

template <typename FT> class UniformSphere : public RandomStep<FT>{
	public:
		void next(int dim, FT* base, FT* bounds, FT* res){
			FT norm;
			norm = 0.;
			for(int j = 0; j < dim; j ++) {
				res[j] = Gauss();
				norm += res[j] * res[j];
			};
			FT radius = *bounds;
			FT a;
			a = radius / sqrt(norm);
			for(int j = 0; j < dim; j ++) {
				res[j] *= a;
			};
			if(base) {
				for(int j = 0; j < dim; j ++) {
					res[j] += base[j];
				};
			};
		};
		
		void GetBounds(int dim,FT r, FT * bounds){
			bounds = &r;
		};
		
		int GetBoundsSize(int dim){
			return sizeof(FT);
		};

	
	private:
		FT Gauss() {
			int SeqLen = 30;
			FT v,u;
			v = 0.;
			for(int i = 0; i < SeqLen; i ++) {
				u = ((FT)rand())/ ((FT)RAND_MAX);
				v += u;
			}	
			return v - (0.5 * ((FT)SeqLen));
		};
};

template <typename FT> class UniformBall : public RandomStep<FT>{
	public:
		void next(int dim, FT* base, FT* bounds, FT* res){
			FT norm;
			norm = 0.;
			for(int j = 0; j < dim; j ++) {
				res[j] = Gauss();
				norm += res[j] * res[j];
			};
			FT radius = *bounds;
			FT a;
			a = ((FT)rand())/ ((FT)RAND_MAX);
			a  *= radius / sqrt(norm) ;
			for(int j = 0; j < dim; j ++) {
				res[j] *= a;
			};
			if(base) {
				for(int j = 0; j < dim; j ++) {
					res[j] += base[j];
				};
			};
		};
		
		void GetBounds(int dim,FT r, FT * bounds){
			bounds = &r;
		};
		
		int GetBoundsSize(int dim){
			return sizeof(FT);
		};

	
	private:
		FT Gauss() {
			int SeqLen = 30;
			FT v,u;
			v = 0.;
			for(int i = 0; i < SeqLen; i ++) {
				u = ((FT)rand())/ ((FT)RAND_MAX);
				v += u;
			}	
			return v - (0.5 * ((FT)SeqLen));
		};
};

#endif
