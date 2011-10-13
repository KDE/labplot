
void SpreadsheetView::initActions(){
	// selection related actions
	action_cut_selection = new KAction(KIcon("edit-cut"), tr("Cu&t"), this);
	action_copy_selection = new KAction(KIcon("edit-copy"), tr("&Copy"), this);
	action_paste_into_selection = new KAction(KIcon("edit-paste"), tr("Past&e"), this);
	action_mask_selection = new KAction(KIcon(""), tr("&Mask","mask selection"), this);
	action_unmask_selection = new KAction(KIcon(""), tr("&Unmask","unmask selection"), this);

	action_set_formula = new KAction(KIcon(""), tr("Assign &Formula"), this);
	action_clear_selection = new KAction(KIcon("edit-clear"), tr("Clea&r","clear selection"), this);
	action_recalculate = new KAction(KIcon(""), tr("Recalculate"), this);
	action_fill_row_numbers = new KAction(KIcon(""), tr("Row Numbers"), this);
	action_fill_random = new KAction(KIcon(""), tr("Random Values"), this);
// 	return;
	//spreadsheet related actions
	action_toggle_comments = new KAction(KIcon(""), QString("Show/Hide comments"), this); // show/hide column comments
	action_select_all = new KAction(KIcon("edit-select-all"), tr("Select All"), this);
	action_add_column = new KAction(KIcon("edit-table-insert-column-left"), tr("&Add Column"), this);
	action_clear_spreadsheet = new KAction(KIcon("edit-clear"), tr("Clear Spreadsheet"), this);
	action_clear_masks = new KAction(KIcon(""), tr("Clear Masks"), this);
	action_sort_spreadsheet = new KAction(KIcon(""), tr("&Sort Spreadsheet"), this);
	action_go_to_cell = new KAction(KIcon(""), tr("&Go to Cell"), this);
	action_dimensions_dialog = new KAction(KIcon(""), tr("&Dimensions", "spreadsheet size"), this);

	// column related actions
	action_insert_columns = new KAction(KIcon(""), tr("&Insert Empty Columns"), this);
	action_remove_columns = new KAction(KIcon(""), tr("Remo&ve Columns"), this);
	action_clear_columns = new KAction(KIcon(""), tr("Clea&r Columns"), this);
	action_add_columns = new KAction(KIcon(""), tr("&Add Columns"), this);
	action_set_as_x = new KAction(KIcon(""), tr("X","plot designation"), this);
	action_set_as_y = new KAction(KIcon(""), tr("Y","plot designation"), this);
	action_set_as_z = new KAction(KIcon(""), tr("Z","plot designation"), this);
	action_set_as_xerr = new KAction(KIcon(""), tr("X Error","plot designation"), this);
	action_set_as_yerr = new KAction(KIcon(""), tr("Y Error","plot designation"), this);
	action_set_as_none = new KAction(KIcon(""), tr("None","plot designation"), this);
	action_normalize_columns = new KAction(KIcon(""), tr("&Normalize Columns"), this);
	action_normalize_selection = new KAction(KIcon(""), tr("&Normalize Selection"), this);
	action_sort_columns = new KAction(KIcon(""), tr("&Sort Columns"), this);
	action_statistics_columns = new KAction(KIcon(""), tr("Column Statisti&cs"), this);

	// row related actions
	action_insert_rows = new KAction(KIcon("edit-table-insert-row-above") ,tr("&Insert Empty Rows"), this);
	action_remove_rows = new KAction(KIcon("edit-table-delete-row"), tr("Remo&ve Rows"), this);
	action_clear_rows = new KAction(KIcon("edit-clear"), tr("Clea&r Rows"), this);
	action_add_rows = new KAction(KIcon("edit-table-insert-row-above"), tr("&Add Rows"), this);
	action_statistics_rows = new KAction(KIcon(""), tr("Row Statisti&cs"), this);
}