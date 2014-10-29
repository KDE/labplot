#include <KLocale>

void SpreadsheetView::initActions(){
	// selection related actions
	action_cut_selection = new KAction(KIcon("edit-cut"), i18n("Cu&t"), this);
	action_copy_selection = new KAction(KIcon("edit-copy"), i18n("&Copy"), this);
	action_paste_into_selection = new KAction(KIcon("edit-paste"), i18n("Past&e"), this);
	action_mask_selection = new KAction(KIcon("edit-node"), i18n("&Mask Selection"), this);
	action_unmask_selection = new KAction(KIcon("format-remove-node"), i18n("&Unmask Selection"), this);

	action_set_formula = new KAction(KIcon(""), i18n("Assign &Formula"), this);
	action_clear_selection = new KAction(KIcon("edit-clear"), i18n("Clea&r Selection"), this);
	action_recalculate = new KAction(KIcon(""), i18n("Recalculate"), this);
	action_fill_row_numbers = new KAction(KIcon(""), i18n("Row Numbers"), this);
	action_fill_random = new KAction(KIcon(""), i18n("Uniform Random Values"), this);
	action_fill_random_nonuniform = new KAction(KIcon(""), i18n("Random Values"), this);
	action_fill_equidistant = new KAction(KIcon(""), i18n("Equidistant Values"), this);
	action_fill_function = new KAction(KIcon(""), i18n("Function Values"), this);
	action_fill_const = new KAction(KIcon(""), i18n("Const Values"), this);

	//spreadsheet related actions
	action_toggle_comments = new KAction(KIcon("document-properties"), i18n("Show Comments"), this);
	action_select_all = new KAction(KIcon("edit-select-all"), i18n("Select All"), this);
	action_add_column = new KAction(KIcon("edit-table-insert-column-left"), i18n("&Add Column"), this);
	action_clear_spreadsheet = new KAction(KIcon("edit-clear"), i18n("Clear Spreadsheet"), this);
	action_clear_masks = new KAction(KIcon("format-remove-node"), i18n("Clear Masks"), this);
	action_sort_spreadsheet = new KAction(KIcon("view-sort-ascending"), i18n("&Sort Spreadsheet"), this);
	action_go_to_cell = new KAction(KIcon("go-jump"), i18n("&Go to Cell"), this);

	// column related actions
	action_insert_columns = new KAction(KIcon("edit-table-insert-column-left"), i18n("&Insert Empty Columns"), this);
	action_remove_columns = new KAction(KIcon("edit-table-delete-column"), i18n("Remo&ve Columns"), this);
	action_clear_columns = new KAction(KIcon("edit-clear"), i18n("Clea&r Columns"), this);
	action_add_columns = new KAction(KIcon("edit-table-insert-column-right"), i18n("&Add Columns"), this);
	action_set_as_x = new KAction(KIcon(""), i18n("X, Plot Designation"), this);
	action_set_as_y = new KAction(KIcon(""), i18n("Y, Plot Designation"), this);
	action_set_as_z = new KAction(KIcon(""), i18n("Z, Plot Designation"), this);
	action_set_as_xerr = new KAction(KIcon(""), i18n("X Error, Plot Designation"), this);
	action_set_as_yerr = new KAction(KIcon(""), i18n("Y Error, Plot Designation"), this);
	action_set_as_none = new KAction(KIcon(""), i18n("None, Plot Designation"), this);
	action_normalize_columns = new KAction(KIcon(""), i18n("&Normalize Columns"), this);
	action_normalize_selection = new KAction(KIcon(""), i18n("&Normalize Selection"), this);
	action_sort_columns = new KAction(KIcon(""), i18n("&Selected Columns"), this);
	action_sort_asc_column = new KAction(KIcon("view-sort-ascending"), i18n("&Ascending"), this);
	action_sort_desc_column = new KAction(KIcon("view-sort-descending"), i18n("&Descending"), this);
	action_statistics_columns = new KAction(KIcon("view-statistics"), i18n("Column Statisti&cs"), this);

	// row related actions
	action_insert_rows = new KAction(KIcon("edit-table-insert-row-above") ,i18n("&Insert Empty Rows"), this);
	action_remove_rows = new KAction(KIcon("edit-table-delete-row"), i18n("Remo&ve Rows"), this);
	action_clear_rows = new KAction(KIcon("edit-clear"), i18n("Clea&r Rows"), this);
	action_add_rows = new KAction(KIcon("edit-table-insert-row-above"), i18n("&Add Rows"), this);
	action_statistics_rows = new KAction(KIcon("view-statistics"), i18n("Row Statisti&cs"), this);
}
