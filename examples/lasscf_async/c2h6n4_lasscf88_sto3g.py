from pyscf import scf
from mrh.tests.lasscf.c2h6n4_struct import structure as struct
from mrh.my_pyscf.mcscf import lasscf_sync_o0 as syn
from mrh.my_pyscf.mcscf import lasscf_async as asyn
from mrh.my_pyscf.mcscf.lasscf_async import old_aa_sync_kernel

mol = struct (1.0, 1.0, 'sto-3g', symmetry=False)
mol.verbose = 5
mol.output = 'c2h6n4_lasscf88_sto3g.log'
mol.build ()
mf = scf.RHF (mol).run ()
las_syn = syn.LASSCF (mf, (4,4), ((4,0),(0,4)), spin_sub=(5,5))
mo = las_syn.localize_init_guess ((list (range (3)), list (range (9,12))), mf.mo_coeff)
las_syn.state_average_(weights=[1,0,0,0,0],
                       spins=[[0,0],[2,0],[-2,0],[0,2],[0,-2]],
                       smults=[[1,1],[3,1],[3,1],[1,3],[1,3]])
las_syn.kernel (mo)
print ("Synchronous calculation converged?", las_syn.converged)

las_asyn = asyn.LASSCF (mf, (4,4), ((4,0),(0,4)), spin_sub=(5,5))
# To fiddle with the optimization parameters of the various subproblems, use
# the "impurity_params" and "relax_params" dictionaries
las_asyn.max_cycle_macro = 50 # by default, all subproblems use this
las_asyn.impurity_params['max_cycle_macro'] = 51 # all fragments
las_asyn.impurity_params[1]['max_cycle_macro'] = 52 # second fragment only (has priority)
las_asyn.relax_params['max_cycle_macro'] = 53 # "flas", the "LASCI step"
# If you have more than two fragments, you can apply specific parameters to orbital relaxations
# between specific pairs of fragments. Addressing specific fragment pairs has priority over
# the global settings above.
las_asyn.relax_params['max_cycle_micro'] = 6 # loses
las_asyn.relax_params[(0,1)]['max_cycle_micro'] = 7 # wins
# However, the old_aa_sync_kernel doesn't relax the active orbitals in a pairwise way, so stuff like
# "relax_params[(0,1)]" is ignored if we patch in the old kernel:
# 
# las_asyn = old_aa_sync_kernel.patch_kernel (las_asyn) # uncomment me to make 6 win

mo = las_asyn.set_fragments_((list (range (3)), list (range (9,12))), mf.mo_coeff)
las_asyn.state_average_(weights=[1,0,0,0,0],
                        spins=[[0,0],[2,0],[-2,0],[0,2],[0,-2]],
                        smults=[[1,1],[3,1],[3,1],[1,3],[1,3]])
las_asyn.kernel (mo)
print ("Asynchronous calculation converged?", las_asyn.converged)
print ("Final state energies:")
print ("{:>16s} {:>16s} {:>16s}".format ("Synchronous", "Asynchronous", "Difference"))
fmt_str = "{:16.9e} {:16.9e} {:16.9e}"
for es, ea in zip (las_syn.e_states, las_asyn.e_states): print (fmt_str.format (es, ea, ea-es))




