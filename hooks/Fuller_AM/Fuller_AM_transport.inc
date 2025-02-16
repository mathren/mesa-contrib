subroutine TSF_Fuller19(id, ierr)
  ! original from https://zenodo.org/records/3228403
  ! see Fuller et al. 2019, https://ui.adsabs.harvard.edu/abs/2019MNRAS.485.3661F/abstract
  integer, intent(in) :: id
  integer, intent(out) :: ierr
  type (star_info), pointer :: s
  integer :: k,j,op_err,nsmooth,nsmootham
  real(dp) :: alpha,shearsmooth,nu_tsf,nu_tsf_t,omegac,omegag,omegaa,&
       omegat,difft,diffm,brunts,bruntsn2,logamnuomega,alphaq

  call star_ptr(id,s,ierr)
  if (ierr /= 0) return

  alpha=1d0
  nsmooth=5
  nsmootham=nsmooth-3
  shearsmooth=1d-30
  op_err = 0

  !Calculate shear at each zone, then calculate TSF torque
  do k=nsmooth+1,s% nz-(nsmooth+1)

     nu_tsf=1d-30
     nu_tsf_t=1d-30
     !Calculate smoothed shear, q= dlnOmega/dlnr
     shearsmooth = s% omega_shear(k)/(2d0*nsmooth+1d0)
     do j=1,nsmooth
        shearsmooth = shearsmooth + (1d0/(2d0*nsmooth+1d0))*( s% omega_shear(k-j) + s% omega_shear(k+j) )
     end do

     diffm =  diffmag(s% rho(k),s% T(k),s% abar(k),s% zbar(k),op_err, id) !Magnetic diffusivity
     difft = 16d0*5.67d-5*pow3(s% T(k))/(3d0*s% opacity(k)*pow2(s% rho(k))*s% Cv(k)) !Thermal diffusivity
     omegaa = s% omega(k)*pow(shearsmooth*s% omega(k)/sqrt(abs(s% brunt_N2(k))),1d0/3d0) !Alfven frequency at saturation, assuming adiabatic instability
     omegat = difft*pow2(sqrt(abs(s% brunt_N2(k)))/(omegaa*s% r(k))) !Thermal damping rate assuming adiabatic instability
     brunts = sqrt(abs( s% brunt_N2_composition_term(k)&
          +(s% brunt_N2(k)-s% brunt_N2_composition_term(k))/(1d0 + omegat/omegaa) )) !Suppress thermal part of brunt
     bruntsn2 = sqrt(abs( s% brunt_N2_composition_term(k)+&
          (s% brunt_N2(k)-s% brunt_N2_composition_term(k))*min(1d0,diffm/difft) )) !Effective brunt for isothermal instability
     brunts = max(brunts,bruntsn2) !Choose max between suppressed brunt and isothermal brunt
     brunts = max(s% omega(k),brunts) !Don't let Brunt be smaller than omega
     omegaa = s% omega(k)*pow(abs(shearsmooth*s% omega(k)/brunts),1d0/3d0) !Recalculate omegaa

     ! Calculate nu_TSF
     if (s% brunt_N2(k) > 0d0) then
        if (pow2(brunts) > 2d0*pow2(shearsmooth)*pow2(s% omega(k))) then
           omegac = 1d0*s% omega(k)*sqrt(brunts/s% omega(k))*pow(diffm/(pow2(s% r(k))*s% omega(k)),0.25d0)  !Critical field strength
           nu_tsf = 5d-1+5d-1*tanh(5d0*log(alpha*omegaa/omegac)) !Suppress AM transport if omega_a<omega_c
           nu_tsf = nu_tsf*pow3(alpha)*s% omega(k)*pow2(s% r(k))*pow2(s% omega(k)/brunts) !nu_omega for revised Tayler instability
        end if
        ! Add TSF enabled by thermal diffusion
        if (pow2(brunts) < 2d0*pow2(shearsmooth)*pow2(s% omega(k))) then
           nu_tsf_t = alpha*abs(shearsmooth)*s% omega(k)*pow2(s% r(k))
        end if
        s% am_nu_omega(k) = s% am_nu_omega(k) + max(nu_tsf,nu_tsf_t) + 1d-1
     end if

  end do


  !Values near inner boundary
  do k=s% nz-nsmooth,s% nz
     nu_tsf=1d-30
     nu_tsf_t=1d-30
     shearsmooth = shearsmooth

     diffm =  diffmag(s% rho(k),s% T(k),s% abar(k),s% zbar(k),op_err, id) !Magnetic diffusivity
     difft = 16d0*5.67d-5*pow3(s% T(k))/(3d0*s% opacity(k)*pow2(s% rho(k))*s% Cv(k)) !Thermal diffusivity
     omegaa = s% omega(k)*pow(shearsmooth*s% omega(k)/sqrt(abs(s% brunt_N2(k))),1d0/3d0) !Alfven frequency at saturation, assuming adiabatic instability
     omegat = difft*pow2(sqrt(abs(s% brunt_N2(k)))/(s% omega(k)*s% r(k))) !Thermal damping rate assuming adiabatic instability
     brunts = sqrt(abs( s% brunt_N2_composition_term(k)&
          +(s% brunt_N2(k)-s% brunt_N2_composition_term(k))/(1d0 + omegat/omegaa) )) !Suppress thermal part of brunt
     bruntsn2 = sqrt(abs( s% brunt_N2_composition_term(k)+&
          (s% brunt_N2(k)-s% brunt_N2_composition_term(k))*min(1d0,diffm/difft) )) !Effective brunt for isothermal instability
     brunts = max(brunts,bruntsn2) !Choose max between suppressed brunt and isothermal brunt
     brunts = max(s% omega(k),brunts) !Don't let Brunt be smaller than omega
     omegaa = s% omega(k)*pow(abs(shearsmooth*s% omega(k)/brunts),1d0/3d0) !Recalculate omegaa

     ! Calculate nu_TSF
     if (s% brunt_N2(k) > 0d0) then
        if (pow2(brunts) > 2d0*pow2(shearsmooth)*pow2(s% omega(k))) then
           omegac = 1d0*s% omega(k)*sqrt(brunts/s% omega(k))*pow(diffm/(pow2(s% r(k))*s% omega(k)),0.25d0)  !Critical field strength
           nu_tsf = 5d-1+5d-1*tanh(5d0*log(alpha*omegaa/omegac)) !Suppress AM transport if omega_a<omega_c
           nu_tsf = nu_tsf*pow3(alpha)*s% omega(k)*pow2(s% r(k))*pow2(s% omega(k)/brunts) !nu_omega for revised Tayler instability
        end if
        ! Add TSF enabled by thermal diffusion
        if (pow2(brunts) < 2d0*pow2(shearsmooth)*pow2(s% omega(k))) then
           nu_tsf_t = alpha*abs(shearsmooth)*s% omega(k)*pow2(s% r(k))
        end if
        s% am_nu_omega(k) = s% am_nu_omega(k) + max(nu_tsf,nu_tsf_t) + 1d-1
     end if
  end do

  !Values near outer boundary
  do k=nsmooth,1
     nu_tsf=1d-30
     nu_tsf_t=1d-30
     shearsmooth = shearsmooth

     diffm =  diffmag(s% rho(k),s% T(k),s% abar(k),s% zbar(k),op_err, id) !Magnetic diffusivity
     difft = 16d0*5.67d-5*pow3(s% T(k))/(3d0*s% opacity(k)*pow2(s% rho(k))*s% Cv(k)) !Thermal diffusivity
     omegaa = s% omega(k)*pow(shearsmooth*s% omega(k)/sqrt(abs(s% brunt_N2(k))),1d0/3d0) !Alfven frequency at saturation, assuming adiabatic instability
     omegat = difft*pow2(sqrt(abs(s% brunt_N2(k)))/(s% omega(k)*s% r(k))) !Thermal damping rate assuming adiabatic instability
     brunts = sqrt(abs( s% brunt_N2_composition_term(k)&
          +(s% brunt_N2(k)-s% brunt_N2_composition_term(k))/(1d0 + omegat/omegaa) )) !Suppress thermal part of brunt
     bruntsn2 = sqrt(abs( s% brunt_N2_composition_term(k)+&
          (s% brunt_N2(k)-s% brunt_N2_composition_term(k))*min(1d0,diffm/difft) )) !Effective brunt for isothermal instability
     brunts = max(brunts,bruntsn2) !Choose max between suppressed brunt and isothermal brunt
     brunts = max(s% omega(k),brunts) !Don't let Brunt be smaller than omega
     omegaa = s% omega(k)*pow(abs(shearsmooth*s% omega(k)/brunts),1d0/3d0) !Recalculate omegaa

     ! Calculate nu_TSF
     if (s% brunt_N2(k) > 0d0) then
        if (pow2(brunts) > 2d0*pow2(shearsmooth)*pow2(s% omega(k))) then
           omegac = 1d0*s% omega(k)*sqrt(brunts/s% omega(k))*pow(diffm/(pow2(s% r(k))*s% omega(k)),0.25d0)  !Critical field strength
           nu_tsf = 5d-1+5d-1*tanh(5d0*log(alpha*omegaa/omegac)) !Suppress AM transport if omega_a<omega_c
           nu_tsf = nu_tsf*pow3(alpha)*s% omega(k)*pow2(s% r(k))*pow2(s% omega(k)/brunts) !nu_omega for revised Tayler instability
        end if
        ! Add TSF enabled by thermal diffusion
        if (pow2(brunts) < 2d0*pow2(shearsmooth)*pow2(s% omega(k))) then
           nu_tsf_t = alpha*abs(shearsmooth)*s% omega(k)*pow2(s% r(k))
        end if
        s% am_nu_omega(k) = s% am_nu_omega(k) + max(nu_tsf,nu_tsf_t) + 1d-1
     end if
  end do

  !Smooth nu_omega
  logamnuomega=-3d1
  do k=nsmootham+1,s% nz-(nsmootham+1)
     !Don't smooth convective diffusivity into non-convective zones
     if (s% mixing_type(k)==1) then
        s% am_nu_omega(k) = s% am_nu_omega(k)
        !Smooth zones if not including a convective zone
     else
        logamnuomega = log10(s% am_nu_omega(k))/(2d0*nsmootham+1d0)
     end if
     do j=1,nsmootham
        !Don't smooth convective diffusivity into non-convective zones
        if (s% mixing_type(k-j)<3.5d0) then
           logamnuomega = log10(s% am_nu_omega(k))
           !Smooth zones if not including a convective zone
        else
           logamnuomega = logamnuomega + (1d0/(2d0*nsmootham+1d0))*log10(s% am_nu_omega(k-j))
        end if
     end do
     do j=1,nsmootham
        !Don't smooth convective diffusivity into non-convective zones
        if (s% mixing_type(k+j)<3.5d0) then
           logamnuomega = logamnuomega
           !Smooth zones if not including a convective zone
        else
           logamnuomega = logamnuomega + (1d0/(2d0*nsmootham+1d0))*log10(s% am_nu_omega(k+j))
        end if
     end do
     s% am_nu_omega(k) = pow(10d0,logamnuomega)
  end do

  !Values near inner boundary
  do k=s% nz-nsmootham,s% nz
     s% am_nu_omega(k) = s% am_nu_omega(k-1)
  end do

  !Values near outer boundary
  do k=nsmootham,1
     s% am_nu_omega(k) = s% am_nu_omega(k-1)
  end do

end subroutine TSF_Fuller19

subroutine TSF_Fuller_Lu22(id, ierr)
  ! original from https://zenodo.org/records/5778001
  ! from Fuller & Lu 2022, https://ui.adsabs.harvard.edu/abs/2022MNRAS.511.3951F/abstract
  integer, intent(in) :: id
  integer, intent(out) :: ierr
  type (star_info), pointer :: s
  integer :: k,j,op_err,nsmooth,nsmootham
  real(dp) :: alpha,shearsmooth,nu_tsf,nu_tsf_t,omegac,omegag,omegaa,omegat,difft,diffm,brunts,bruntsn2

  call star_ptr(id,s,ierr)
  if (ierr /= 0) return

  nsmooth=1!
  nsmootham=1
  shearsmooth=1d-30
  op_err = 0
  alpha = 2.5d-1

  !Calculate shear at each zone, then calculate TSF torque
  do k=nsmooth+1,s% nz-(nsmooth+1)

     nu_tsf=1d-30
     nu_tsf_t=1d-30
     !Calculate smoothed shear, q= dlnOmega/dlnr
     shearsmooth = s% omega_shear(k)/(2.*nsmooth+1.)
     do j=1,nsmooth
        shearsmooth = shearsmooth + (1./(2.*nsmooth+1.))*( s% omega_shear(k-j) + s% omega_shear(k+j) )
     end do

     diffm =  diffmag(s% rho(k),s% T(k),s% abar(k),s% zbar(k),op_err, id) !Magnetic diffusivity
     difft = 16d0*5.67d-5*(s% T(k))**3/(3d0*s% opacity(k)*(s% rho(k))**2*s% Cp(k)) !Thermal diffusivity
     omegaa = s% omega(k)*(shearsmooth*s% omega(k)/sqrt(abs(s% brunt_N2(k))))**(1./3.) !Alfven frequency at saturation, assuming adiabatic instability
     omegat = difft*pow2(sqrt(abs(s% brunt_N2(k)))/(omegaa*s% r(k))) !Thermal damping rate assuming adiabatic instability
     brunts = sqrt(abs( s% brunt_N2_composition_term(k)+(s% brunt_N2(k)-s% brunt_N2_composition_term(k))/(1d0 + omegat/omegaa) )) !Suppress thermal part of brunt
     bruntsn2 = sqrt(abs( s% brunt_N2_composition_term(k)+(s% brunt_N2(k)-s% brunt_N2_composition_term(k))*min(1d0,diffm/difft) )) !Effective brunt for isothermal instability
     brunts = max(brunts,bruntsn2) !Choose max between suppressed brunt and isothermal brunt
     brunts = max(s% omega(k),brunts) !Don't let Brunt be smaller than omega
     omegaa = s% omega(k)*abs(shearsmooth*s% omega(k)/brunts)**(1./3.) !Recalculate omegaa

     ! Calculate nu_TSF
     if (s% brunt_N2(k) > 0.) then
        if (pow2(brunts) > 2.*pow2(shearsmooth)*pow2(s% omega(k))) then
           omegac = 1d0*s% omega(k)*((brunts/s% omega(k))**0.5)*(diffm/(pow2(s% r(k))*s% omega(k)))**0.25  !Critical field strength
           !nu_tsf = 5d-1+5d-1*tanh(10d0*log(alpha*omegaa/omegac)) !Suppress AM transport if omega_a<omega_c
           !nu_tsf = nu_tsf*alpha**3*s% omega(k)*pow2(s% r(k))*(s% omega(k)/brunts)**2 !nu_omega for revised Tayler instability
           nu_tsf = alpha**3*pow2(s% r(k))*s% omega(k)*(s% omega(k)/brunts)**2 !nu_omega for revised Tayler instability

        end if
        ! Add TSF enabled by thermal diffusion
        if (pow2(brunts) < 2.*pow2(shearsmooth)*pow2(s% omega(k))) then
           nu_tsf_t = alpha*abs(shearsmooth)*s% omega(k)*pow2(s% r(k))
        end if
        s% am_nu_omega(k) = s% am_nu_omega(k) + max(nu_tsf,nu_tsf_t) + 1d-1
     end if
  end do

end subroutine TSF_Fuller_Lu22

real(dp) function diffmag(rho, T, abar, zbar, ierr, id)
  ! Written by S.-C. Yoon, Oct. 10, 2003
  ! Electrical conductivity according to Spitzer 1962
  ! See also Wendell et al. 1987, ApJ 313:284
  integer, intent(in) :: id
  real(dp), intent(in) :: rho, T, abar, zbar
  integer, intent(out) :: ierr
  real(dp) :: xmagfmu, xmagft, xmagfdif, xmagfnu, &
       xkap, xgamma, xlg, xsig1, xsig2, xsig3, xxx, ffff, xsig, &
       xeta
  ! initialize
  diffmag = 0.0d0
  if (ierr /= 0) return

  xgamma = 0.2275d0*zbar*zbar*pow(rho*1d-6/abar,1d0/3d0)*1d8/T
  xlg = log10(xgamma)
  if (xlg < -1.5d0) then
     xsig1 = sige1(zbar,T,xgamma)
     xsig = xsig1
  else if (xlg >= -1.5d0 .and. xlg <= 0d0) then
     xxx = (xlg + 0.75d0)*4d0/3d0
     ffff = 0.25d0*(2d0-3d0*xxx + xxx*xxx*xxx)
     xsig1 = sige1(zbar,T,xgamma)

     xsig2 = sige2(T,rho,zbar,ierr, id)
     if (ierr /= 0) return

     xsig = (1d0-ffff)*xsig2 + ffff*xsig1
  else if (xlg > 0d0 .and. xlg < 0.5d0) then
     xsig2 = sige2(T,rho,zbar,ierr, id)
     if (ierr /= 0) return

     xsig = xsig2
  else if (xlg >= 0.5d0 .and. xlg < 1d0) then
     xxx = (xlg-0.75d0)*4d0
     ffff = 0.25d0*(2d0-3d0*xxx + xxx*xxx*xxx)
     xsig2 = sige2(T,rho,zbar,ierr, id)
     if (ierr /= 0) return

     xsig3 = sige3(zbar,T,xgamma)
     xsig = (1d0-ffff)*xsig3 + ffff*xsig2
  else
     xsig3 = sige3(zbar,T,xgamma)
     xsig = xsig3
  endif

  diffmag = 7.1520663d19/xsig ! magnetic diffusivity

end function diffmag


! Helper functions

real(dp) function sige1(z,t,xgamma)
  ! Written by S.-C. Yoon, Oct. 10, 2003
  ! Electrical conductivity according to Spitzer 1962
  ! See also Wendell et al. 1987, ApJ 313:284
  real(dp), intent(in) :: z, t, xgamma
  real(dp) :: etan, xlambda,f
  if (t >= 4.2d5) then
     f = sqrt(4.2d5/t)
  else
     f = 1d0
  end if
  xlambda = sqrt(3d0*z*z*z)*pow(xgamma,-1.5d0)*f + 1d0
  etan = 3d11*z*log(xlambda)*pow(t,-1.5d0)             ! magnetic diffusivity
  etan = etan/(1d0-1.20487d0*exp(-1.0576d0*pow(z,0.347044d0))) ! correction: gammae
  sige1 = clight*clight/(4d0*pi*etan)                    ! sigma = c^2/(4pi*eta)
end function sige1


real(dp) function sige2(T, rho, zbar, ierr, id)
  ! writen by S.-C. YOON Oct. 10, 2003
  ! electrical conductivity using conductive opacity
  ! see Wendell et al. 1987 ApJ 313:284
  use kap_lib, only: kap_get_elect_cond_opacity
  integer, intent(in) :: id
  real(dp), intent(in) :: t,rho,zbar
  integer, intent(out) :: ierr
  real(dp) :: kap, dlnkap_dlnRho, dlnkap_dlnT
  type (star_info), pointer :: s
  call star_ptr(id, s, ierr)

  call kap_get_elect_cond_opacity( &
       s% kap_handle, &
       zbar, log10(rho), log10(T),  &
       kap, dlnkap_dlnRho, dlnkap_dlnT, ierr)
  sige2 = 1.11d9*T*T/(rho*kap)
end function sige2

real(dp) function sige3(z, t, xgamma)
  ! writen by S.-C. YOON Oct. 10, 2003
  ! electrical conductivity in degenerate matter,
  ! according to Nandkumar & Pethick (1984)
  real(dp), intent(in) :: z, t, xgamma
  real(dp) :: rme, rm23, ctmp, xi
  rme = 8.5646d-23*t*t*t*xgamma*xgamma*xgamma/pow5(z)  ! rme = rho6/mue
  rm23 = pow(rme,2d0/3d0)
  ctmp = 1d0 + 1.018d0*rm23
  xi= sqrt(3.14159d0/3d0)*log(z)/3d0 + 2d0*log(1.32d0+2.33d0/sqrt(xgamma))/3d0-0.484d0*rm23/ctmp
  sige3 = 8.630d21*rme/(z*ctmp*xi)
end function sige3
