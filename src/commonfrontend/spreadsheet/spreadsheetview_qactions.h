
void SpreadsheetView::initActions(){
	/*
	QIcon * icon_temp;

	// selection related actions
	action_cut_selection = new QAction(QIcon(QPixmap(":/cut.xpm")), tr("Cu&t"), this);
	actionManager()->addAction(action_cut_selection, "cut_selection");

	action_copy_selection = new QAction(QIcon(QPixmap(":/copy.xpm")), tr("&Copy"), this);
	actionManager()->addAction(action_copy_selection, "copy_selection");

	action_paste_into_selection = new QAction(QIcon(QPixmap(":/paste.xpm")), tr("Past&e"), this);
	actionManager()->addAction(action_paste_into_selection, "paste_into_selection"); 

	action_mask_selection = new QAction(QIcon(QPixmap(":/mask.xpm")), tr("&Mask","mask selection"), this);
	actionManager()->addAction(action_mask_selection, "mask_selection"); 

	action_unmask_selection = new QAction(QIcon(QPixmap(":/unmask.xpm")), tr("&Unmask","unmask selection"), this);
	actionManager()->addAction(action_unmask_selection, "unmask_selection"); 

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/fx.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/fx.png"));
	action_set_formula = new QAction(*icon_temp, tr("Assign &Formula"), this);
	actionManager()->addAction(action_set_formula, "set_formula"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/clear.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/clear.png"));
	action_clear_selection = new QAction(*icon_temp, tr("Clea&r","clear selection"), this);
	actionManager()->addAction(action_clear_selection, "clear_selection"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/recalculate.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/recalculate.png"));
	action_recalculate = new QAction(*icon_temp, tr("Recalculate"), this);
	actionManager()->addAction(action_recalculate, "recalculate"); 
	delete icon_temp;

	action_fill_row_numbers = new QAction(QIcon(QPixmap(":/rowNumbers.xpm")), tr("Row Numbers"), this);
	actionManager()->addAction(action_fill_row_numbers, "fill_row_numbers"); 

	action_fill_random = new QAction(QIcon(QPixmap(":/randomNumbers.xpm")), tr("Random Values"), this);
	actionManager()->addAction(action_fill_random, "fill_random"); 
	
	//spreadsheet related actions
	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/table_header.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/table_header.png"));
	action_toggle_comments = new QAction(*icon_temp, QString("Show/Hide comments"), this); // show/hide column comments
	actionManager()->addAction(action_toggle_comments, "toggle_comments"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/table_options.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/table_options.png"));
	action_toggle_tabbar = new QAction(*icon_temp, QString("Show/Hide Controls"), this); // show/hide control tabs
	actionManager()->addAction(action_toggle_tabbar, "toggle_tabbar"); 
	delete icon_temp;

	action_formula_mode = new QAction(tr("Formula Edit Mode"), this);
	action_formula_mode->setCheckable(true);
	actionManager()->addAction(action_formula_mode, "formula_mode"); 

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/select_all.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/select_all.png"));
	action_select_all = new QAction(*icon_temp, tr("Select All"), this);
	actionManager()->addAction(action_select_all, "select_all"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/add_column.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/add_column.png"));
	action_add_column = new QAction(*icon_temp, tr("&Add Column"), this);
	actionManager()->addAction(action_add_column, "add_column"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/clear_table.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/clear_table.png"));
	action_clear_spreadsheet = new QAction(*icon_temp, tr("Clear Spreadsheet"), this);
	actionManager()->addAction(action_clear_spreadsheet, "clear_spreadsheet"); 
	delete icon_temp;

	action_clear_masks = new QAction(QIcon(QPixmap(":/unmask.xpm")), tr("Clear Masks"), this);
	actionManager()->addAction(action_clear_masks, "clear_masks"); 

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/sort.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/sort.png"));
	action_sort_spreadsheet = new QAction(*icon_temp, tr("&Sort Spreadsheet"), this);
	actionManager()->addAction(action_sort_spreadsheet, "sort_spreadsheet"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/go_to_cell.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/go_to_cell.png"));
	action_go_to_cell = new QAction(*icon_temp, tr("&Go to Cell"), this);
	actionManager()->addAction(action_go_to_cell, "go_to_cell"); 
	delete icon_temp;

	action_dimensions_dialog = new QAction(QIcon(QPixmap(":/resize.xpm")), tr("&Dimensions", "spreadsheet size"), this);
	actionManager()->addAction(action_dimensions_dialog, "dimensions_dialog"); 

	// column related actions
	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/insert_column.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/insert_column.png"));
	action_insert_columns = new QAction(*icon_temp, tr("&Insert Empty Columns"), this);
	actionManager()->addAction(action_insert_columns, "insert_columns"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/remove_column.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/remove_column.png"));
	action_remove_columns = new QAction(*icon_temp, tr("Remo&ve Columns"), this);
	actionManager()->addAction(action_remove_columns, "remove_columns"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/clear_column.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/clear_column.png"));
	action_clear_columns = new QAction(*icon_temp, tr("Clea&r Columns"), this);
	actionManager()->addAction(action_clear_columns, "clear_columns"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/add_columns.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/add_columns.png"));
	action_add_columns = new QAction(*icon_temp, tr("&Add Columns"), this);
	actionManager()->addAction(action_add_columns, "add_columns"); 
	delete icon_temp;

	action_set_as_x = new QAction(QIcon(QPixmap()), tr("X","plot designation"), this);
	actionManager()->addAction(action_set_as_x, "set_as_x"); 

	action_set_as_y = new QAction(QIcon(QPixmap()), tr("Y","plot designation"), this);
	actionManager()->addAction(action_set_as_y, "set_as_y"); 

	action_set_as_z = new QAction(QIcon(QPixmap()), tr("Z","plot designation"), this);
	actionManager()->addAction(action_set_as_z, "set_as_z"); 

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/x_error.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/x_error.png"));
	action_set_as_xerr = new QAction(*icon_temp, tr("X Error","plot designation"), this);
	actionManager()->addAction(action_set_as_xerr, "set_as_xerr"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/y_error.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/y_error.png"));
	action_set_as_yerr = new QAction(*icon_temp, tr("Y Error","plot designation"), this);
	actionManager()->addAction(action_set_as_yerr, "set_as_yerr"); 
	delete icon_temp;

	action_set_as_none = new QAction(QIcon(QPixmap()), tr("None","plot designation"), this);
	actionManager()->addAction(action_set_as_none, "set_as_none"); 

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/normalize.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/normalize.png"));
	action_normalize_columns = new QAction(*icon_temp, tr("&Normalize Columns"), this);
	actionManager()->addAction(action_normalize_columns, "normalize_columns"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/normalize.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/normalize.png"));
	action_normalize_selection = new QAction(*icon_temp, tr("&Normalize Selection"), this);
	actionManager()->addAction(action_normalize_selection, "normalize_selection"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/sort.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/sort.png"));
	action_sort_columns = new QAction(*icon_temp, tr("&Sort Columns"), this);
	actionManager()->addAction(action_sort_columns, "sort_columns"); 
	delete icon_temp;

	action_statistics_columns = new QAction(QIcon(QPixmap(":/col_stat.xpm")), tr("Column Statisti&cs"), this);
	actionManager()->addAction(action_statistics_columns, "statistics_columns"); 

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/column_format_type.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/column_format_type.png"));
	action_type_format = new QAction(*icon_temp, tr("Change &Type && Format"), this);
	actionManager()->addAction(action_type_format, "type_format"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/column_description.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/column_description.png"));
	action_edit_description = new QAction(*icon_temp, tr("Edit Column &Description"), this);
	actionManager()->addAction(action_edit_description, "edit_description"); 
	delete icon_temp;

	// row related actions
	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/insert_row.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/insert_row.png"));
	action_insert_rows = new QAction(*icon_temp ,tr("&Insert Empty Rows"), this);
	actionManager()->addAction(action_insert_rows, "insert_rows"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/remove_row.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/remove_row.png"));
	action_remove_rows = new QAction(*icon_temp, tr("Remo&ve Rows"), this);
	actionManager()->addAction(action_remove_rows, "remove_rows"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/clear_row.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/clear_row.png"));
	action_clear_rows = new QAction(*icon_temp, tr("Clea&r Rows"), this);
	actionManager()->addAction(action_clear_rows, "clear_rows"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/add_rows.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/add_rows.png"));
	action_add_rows = new QAction(*icon_temp, tr("&Add Rows"), this);
	actionManager()->addAction(action_add_rows, "add_rows"); 
	delete icon_temp;

	action_statistics_rows = new QAction(QIcon(QPixmap(":/stat_rows.xpm")), tr("Row Statisti&cs"), this);
	actionManager()->addAction(action_statistics_rows, "statistics_rows"); 
	*/
}