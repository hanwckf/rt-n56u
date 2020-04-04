
unsigned int
internal_function
_dl_nios2_get_gp_value (struct link_map *main_map)
{ 
  ElfW(Dyn) *dyn = main_map->l_ld;
  for (dyn = main_map->l_ld; dyn->d_tag != DT_NULL; ++dyn)
    if (dyn->d_tag == DT_NIOS2_GP)
      return (unsigned int)(dyn->d_un.d_ptr);
  return 0;
}

