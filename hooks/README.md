# ``Style Suggestions``

## Overview

Routines and data can go anywhere in a contribution directory, including in subfolders.
You just need to be sure to point to the correct locations in your `run_star_extras.f`.

Please include
``
This contribution is maintained by Your Name/GitHub ID
``
in your `README.md` so that your contribution has a contact and known maintainer.

## Usage

It is helpful to include example directories showing how your contribution is used.
Depending on which parts of MESA your contribution touches you may want to include examples for the star, rsp, and/or astero modules.

Typically an example directory contains whatever inlists are needed as well as a `src` directory containing the sample `run_star_extras.f`.
In your `run_star_extras.f`, around the `contains` statement you'll need

````Fortran
      include 'contribution_directory/any_data_to_include.inc'

      contains

      include 'contribution_directory/routines.dek'
      include 'contribution_directory/more_routines/inc'
````

If you have chosen to put your routines and data in subfolders, just amend the paths above accordingly.

Also, if your contribution makes use of any MESA hooks you'll need to point those hooks in extras_controls:

````Fortran
      subroutine extras_controls(id, ierr)
         ...
         call hydro_Ttau_setup(id, ierr)
         ...
         s% other_gradr_factor => hydro_Ttau_gradr_factor
         s% other_surface_PT => hydro_Ttau_surface_PT
         ...
      end subroutine extras_controls
````

Please document all such changes users need to make in order to use your contribution.

## Extra history and profile columns

Sometimes users will want to use multiple contributions in their work.
Each contribution might have routines for adding history and/or profile columns.
To make it easier for users to combine these routines, we recommend a call signature of the form

````Fortran
      subroutine my_contribution_extra_history_columns(id, ierr, array, names, index, ...)
         integer, intent(in) :: id
         integer, intent(out) :: ierr, index
         real(dp), dimension(:), intent(out) :: array
         character (len=maxlen_history_column_name), dimension(:) :: names
         type (star_info), pointer :: s

         call star_ptr(id, s, ierr)
         if (ierr /= 0) return

         ! Calculate new history columns         
         ...

         ! Insert them into the array starting at index
         array(index) = first_column
         names(index) = 'first column'
         array(index+1) = second_column
         names(index+1) = 'second column'
         ...

         ! Increment index for the next routine
         index = index + my_contribution_how_many_extra_history_columns(id)
         
      end subroutine my_contribution_extra_history_columns
      
      subroutine my_contribution_extra_profile_columns(id, ierr, array, name_array, index, ...)
         integer, intent(in) :: id
         integer, intent(out) :: ierr, index
         real(dp), dimension(:,:), intent(out) :: array
         character (len=maxlen_history_column_name), dimension(:) :: names
         integer :: k
         type (star_info), pointer :: s

         call star_ptr(id, s, ierr)
         if (ierr /= 0) return

         ! Calculate new history columns         
         ...

         ! Insert them into the array starting at index
         do k=1,s\%nz
             array(k,index) = first_column(k)
             array(k,index+1) = second_column(k)
             ...
         end do
         names(index) = 'first column'
         names(index+1) = 'second column'
	...
	
         ! Increment index for the next routine
         index = index + my_contribution_how_many_extra_profile_columns(id)
         
      end subroutine my_contribution_extra_profile_columns
      
````

If your routines follow this signature, then a user can incorporate your routines alongside other contributions as follows:

````Fortran
      integer function how_many_extra_history_columns(id)
         integer, intent(in) :: id
         integer :: ierr
         type (star_info), pointer :: s
         ierr = 0
         call star_ptr(id, s, ierr)
         if (ierr /= 0) return
         how_many_extra_history_columns = 0
         how_many_extra_history_columns = how_many_extra_history_columns + contribution1_how_many_extra_history_columns(id)
         how_many_extra_history_columns = how_many_extra_history_columns + contribution2_how_many_extra_history_columns(id)
         how_many_extra_history_columns = how_many_extra_history_columns + contribution3_how_many_extra_history_columns(id)
         ...

      end function how_many_extra_history_columns
      
      
      subroutine data_for_extra_history_columns(id, n, names, vals, ierr)
         integer, intent(in) :: id, n
         character (len=maxlen_history_column_name) :: names(n)
         real(dp) :: vals(n)
         integer, intent(out) :: ierr
         integer :: j
         type (star_info), pointer :: s
         ierr = 0
         call star_ptr(id, s, ierr)
         if (ierr /= 0) return
         
         ! note: do NOT add the extras names to history_columns.list
         ! the history_columns.list is only for the built-in history column options.
         ! it must not include the new column names you are adding here.
         
         j = 1
         call contribution1_extra_history_columns(id, ierr, vals, names, j)
         call contribution2_extra_history_columns(id, ierr, vals, names, j)
         call contribution3_extra_history_columns(id, ierr, vals, names, j)
	...
	
      end subroutine data_for_extra_history_columns
````

and similarly for profile columns.