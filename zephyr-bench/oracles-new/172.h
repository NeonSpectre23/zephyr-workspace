static inline bool nvmem_cell_is_read_only(const struct nvmem_cell *cell)
{
	return cell->read_only;
}