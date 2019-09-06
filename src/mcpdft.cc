#include <armadillo>
#include <iostream>
#include <fstream>

#include "mcpdft.h"

namespace mcpdft {

   MCPDFT::MCPDFT()  { common_init(); }
   MCPDFT::~MCPDFT() {}

   void MCPDFT::common_init() {

       read_grids_from_file();
       read_orbitals_from_file();
       read_energies_from_file(); 
       read_opdm_from_file();
       // read_cmat_from_file();
   }

   void MCPDFT::build_rho() {
      
      int nbfs = get_nbfs();
      size_t npts = get_npts();

      arma::mat phi(get_phi());
      arma::mat D1a(get_D1a());
      arma::mat D1b(get_D1b());
      
      arma::vec rhoa(npts, arma::fill::zeros);
      arma::vec rhob(npts, arma::fill::zeros);
      arma::vec rho(npts, arma::fill::zeros);

      for(int p = 0; p < npts; p++) {
         double tempa = 0.0;
         double tempb = 0.0;
         for(int mu = 0; mu < nbfs; mu++) {
            for(int nu = 0; nu < nbfs; nu++) {
               tempa += D1a(mu, nu) * phi(p, mu) * phi(p, nu);
               tempb += D1b(mu, nu) * phi(p, mu) * phi(p, nu);
	    }
	 }
	 rhoa(p) = tempa;
	 rhob(p) = tempb;
	 rho(p) = rhoa(p) + rhob(p);
      }
      set_rhoa(rhoa);
      set_rhob(rhob);
      set_rho(rho);
   }

   void MCPDFT::build_pi(const arma::mat &D2ab) {

        int nbfs = get_nbfs();
        size_t npts = get_npts();

        arma::vec temp(npts);
        arma::mat phi(get_phi());

        for (int p = 0; p < npts; p++) {
                double dum = 0.0;
                // pi(r,r) = D(mu,nu; lambda,sigma) * phi(r,mu) * phi(r,nu) * phi(r,lambda) * phi(r,sigma)
                for (int mu = 0; mu < nbfs; mu++) {
                    for (int nu = 0; nu < nbfs; nu++) {
                        for (int lambda = 0; lambda < nbfs; lambda++) {
                            for (int sigma = 0; sigma < nbfs; sigma++) {
                                dum += phi(p, mu) * phi(p, nu) * phi(p, lambda) * phi(p, sigma) * D2ab(nu*nbfs+mu, sigma*nbfs+lambda);
                            }
                        }
                    }
                }
                temp(p) = dum;
        }
        set_pi(temp);
   }

   void MCPDFT::build_R() {

        double tol = 1.0e-20;
        size_t npts = get_npts();

        arma::vec temp(npts);
        arma::vec pi(get_pi());
        arma::vec rho(get_rho());

        for (int p = 0; p < npts; p++) {
            temp(p) = 4.0 * pi(p) / ( rho(p) * rho(p) );
        }
        set_R(temp);
   }

   void MCPDFT::build_opdm() {

      // fetching the number of basis functions
      int nbfs;
      nbfs = get_nbfs();

      // getting the AO->MO transformation matrix C
      arma::mat ca(get_cmat());
      arma::mat cb(get_cmat());

      // building the 1-electron reduced density matrices (1RDMs)
      arma::mat D1a(nbfs, nbfs, arma::fill::zeros);
      arma::mat D1b(nbfs, nbfs, arma::fill::zeros);
      for (int mu = 0; mu < nbfs; mu++) { 
          for (int nu = 0; nu < nbfs; nu++) { 
              double duma = 0.0;
              double dumb = 0.0;
              for (int i = 0; i < nbfs/2; i++) { 
                   duma += ca(mu, i) * ca(nu, i);
                   dumb += cb(mu, i) * cb(nu, i);
              }
	      D1a(mu, nu) = duma;
              D1b(mu, nu) = dumb;
	  }
      }
      // D1a(0,0) = 1.0;
      // D1b(0,0) = 1.0;
      D1a.print("D1a = ");
      D1b.print("D1b = ");
      set_D1a(D1a);
      set_D1b(D1b);
   }

   void MCPDFT::build_tpdm() {

      // fetching the number of basis functions
      int nbfs;
      nbfs = get_nbfs();

      arma::mat D1a(get_D1a());
      arma::mat D1b(get_D1b());

      arma::mat D2ab(nbfs*nbfs, nbfs*nbfs, arma::fill::zeros);
      D2ab = arma::kron(D1a,D1b);
      // D2ab.print("D2ab = ");
      set_D2ab(D2ab);
   }

   size_t MCPDFT::get_npts() const { return npts_; }
   int    MCPDFT::get_nbfs() const { return nbfs_; }
   arma::vec MCPDFT::get_w() const { return w_; }
   arma::vec MCPDFT::get_x() const { return x_; }
   arma::vec MCPDFT::get_y() const { return y_; }
   arma::vec MCPDFT::get_z() const { return z_; }
   arma::mat MCPDFT::get_phi() const { return phi_; }
   double MCPDFT::get_eref() const { return eref_; }
   double MCPDFT::get_eclass()  const { return eclass_; }
   arma::mat MCPDFT::get_cmat() const { return cmat_; }
   arma::mat MCPDFT::get_D1a()  const { return D1a_ ; }
   arma::mat MCPDFT::get_D1b()  const { return D1b_ ; }
   arma::mat MCPDFT::get_D2ab()  const { return D2ab_ ; }
   arma::vec MCPDFT::get_rhoa() const { return rho_a_; }
   arma::vec MCPDFT::get_rhob() const { return rho_b_; }
   arma::vec MCPDFT::get_rho() const { return rho_; }
   arma::vec MCPDFT::get_pi() const { return pi_; }
   arma::vec MCPDFT::get_R() const { return R_; }

   void MCPDFT::set_npts(const size_t npts) { npts_ = npts; }
   void MCPDFT::set_nbfs(const int nbfs)    { nbfs_ = nbfs; }
   void MCPDFT::set_w(const arma::vec &w) { w_ = w; }
   void MCPDFT::set_x(const arma::vec &x) { x_ = x; }
   void MCPDFT::set_y(const arma::vec &y) { y_ = y; }
   void MCPDFT::set_z(const arma::vec &z) { z_ = z; }
   void MCPDFT::set_phi(const arma::mat &phi) { phi_ = phi; }
   void MCPDFT::set_eref(const double eref) { eref_ = eref; }
   void MCPDFT::set_eclass(const double eclass) { eclass_ = eclass; }
   void MCPDFT::set_cmat(const arma::mat &cmat) { cmat_ = cmat; }
   void MCPDFT::set_D1a(const arma::mat &D1a) { D1a_ = D1a; }
   void MCPDFT::set_D1b(const arma::mat &D1b) { D1b_ = D1b; }
   void MCPDFT::set_D2ab(const arma::mat &D2ab) { D2ab_ = D2ab; }
   void MCPDFT::set_rhoa(const arma::vec &rhoa) { rho_a_ = rhoa; }
   void MCPDFT::set_rhob(const arma::vec &rhob) { rho_b_ = rhob; }
   void MCPDFT::set_rho(const arma::vec &rho) { rho_ = rho; }
   void MCPDFT::set_pi(const arma::vec &pi) { pi_ = pi; }
   void MCPDFT::set_R(const arma::vec &R) { R_ = R; }

}