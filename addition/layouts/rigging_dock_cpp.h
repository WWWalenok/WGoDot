#ifndef __LAYOUT__CPP
#define __LAYOUT__CPP
///
/// Autogen using tscn_parser.py by WWWalenok v 0.02
/// Autogen based on ./addition/control.tscn
/// 2023-12-09 00:50:21.545893
///

	v_box_container = memnew(VBoxContainer);
	h_box_container1 = memnew(HBoxContainer);
	button_select_obj = memnew(Button);
	line_edit_select_obj = memnew(LineEdit);
	h_box_container2 = memnew(HBoxContainer);
	button_select_skeleton = memnew(Button);
	line_edit_select_skeleton = memnew(LineEdit);
	h_box_container3 = memnew(HBoxContainer);
	button_select_state = memnew(Button);
	line_edit_select_state = memnew(LineEdit);
	button_reset_state = memnew(Button);
	h_box_container5 = memnew(HBoxContainer);
	button_export_skeleton = memnew(Button);
	line_edit_export_skeleton = memnew(LineEdit);
	h_box_container4 = memnew(HBoxContainer);
	button_select_anim_folder = memnew(Button);
	line_edit_select_anim_folder = memnew(LineEdit);
	h_box_container6 = memnew(HBoxContainer);
	button_export_animation = memnew(Button);
	line_edit_export_animation = memnew(LineEdit);
	h_box_container7 = memnew(HBoxContainer);
	button_generate = memnew(Button);
	button_generate_animation = memnew(Button);
	h_box_container8 = memnew(HBoxContainer);
	button_select2dtexture = memnew(Button);
	line_edit_select2dtexture = memnew(LineEdit);
	h_box_container10 = memnew(HBoxContainer);
	button_select_skeleton2d = memnew(Button);
	line_edit_select_skeleton2d = memnew(LineEdit);
	h_box_container9 = memnew(HBoxContainer);
	button_generate2d = memnew(Button);
	button_save_viewport = memnew(Button);


//================== this ==================
	this->set_name("Rigging");
	this->set_anchors_preset(Control::PRESET_FULL_RECT);
	this->set_anchor(SIDE_RIGHT, 1.0);
	this->set_anchor(SIDE_BOTTOM, 1.0);
	this->set_h_grow_direction(Control::GROW_DIRECTION_BOTH);
	this->set_v_grow_direction(Control::GROW_DIRECTION_BOTH);
//================== v_box_container ==================
	v_box_container->set_name("VBoxContainer");
	this->add_child(v_box_container);
	v_box_container->set_anchors_preset(Control::PRESET_FULL_RECT);
	v_box_container->set_anchor(SIDE_RIGHT, 1.0);
	v_box_container->set_anchor(SIDE_BOTTOM, 1.0);
	v_box_container->set_h_grow_direction(Control::GROW_DIRECTION_BOTH);
	v_box_container->set_v_grow_direction(Control::GROW_DIRECTION_BOTH);
//================== h_box_container1 ==================
	h_box_container1->set_name("HBoxContainer1");
	v_box_container->add_child(h_box_container1);
//================== button_select_obj ==================
	button_select_obj->set_name("ButtonSelectObj");
	h_box_container1->add_child(button_select_obj);
	button_select_obj->set_custom_minimum_size(Vector2(200, 0));
	button_select_obj->set_text("Select Object file");
//================== line_edit_select_obj ==================
	line_edit_select_obj->set_name("LineEditSelectObj");
	h_box_container1->add_child(line_edit_select_obj);
	line_edit_select_obj->set_custom_minimum_size(Vector2(200, 0));
	line_edit_select_obj->set_editable(false);
//================== h_box_container2 ==================
	h_box_container2->set_name("HBoxContainer2");
	v_box_container->add_child(h_box_container2);
//================== button_select_skeleton ==================
	button_select_skeleton->set_name("ButtonSelectSkeleton");
	h_box_container2->add_child(button_select_skeleton);
	button_select_skeleton->set_custom_minimum_size(Vector2(200, 0));
	button_select_skeleton->set_text("Select Skeleton file");
//================== line_edit_select_skeleton ==================
	line_edit_select_skeleton->set_name("LineEditSelectSkeleton");
	h_box_container2->add_child(line_edit_select_skeleton);
	line_edit_select_skeleton->set_custom_minimum_size(Vector2(200, 0));
	line_edit_select_skeleton->set_editable(false);
//================== h_box_container3 ==================
	h_box_container3->set_name("HBoxContainer3");
	v_box_container->add_child(h_box_container3);
//================== button_select_state ==================
	button_select_state->set_name("ButtonSelectState");
	h_box_container3->add_child(button_select_state);
	button_select_state->set_custom_minimum_size(Vector2(200, 0));
	button_select_state->set_text("Select State");
//================== line_edit_select_state ==================
	line_edit_select_state->set_name("LineEditSelectState");
	h_box_container3->add_child(line_edit_select_state);
	line_edit_select_state->set_custom_minimum_size(Vector2(95, 0));
	line_edit_select_state->set_editable(false);
//================== button_reset_state ==================
	button_reset_state->set_name("ButtonResetState");
	h_box_container3->add_child(button_reset_state);
	button_reset_state->set_custom_minimum_size(Vector2(95, 0));
	button_reset_state->set_text("Reset");
//================== h_box_container5 ==================
	h_box_container5->set_name("HBoxContainer5");
	v_box_container->add_child(h_box_container5);
//================== button_export_skeleton ==================
	button_export_skeleton->set_name("ButtonExportSkeleton");
	h_box_container5->add_child(button_export_skeleton);
	button_export_skeleton->set_custom_minimum_size(Vector2(200, 0));
	button_export_skeleton->set_text("Export Skeleton");
//================== line_edit_export_skeleton ==================
	line_edit_export_skeleton->set_name("LineEditExportSkeleton");
	h_box_container5->add_child(line_edit_export_skeleton);
	line_edit_export_skeleton->set_custom_minimum_size(Vector2(200, 0));
//================== h_box_container4 ==================
	h_box_container4->set_name("HBoxContainer4");
	v_box_container->add_child(h_box_container4);
//================== button_select_anim_folder ==================
	button_select_anim_folder->set_name("ButtonSelectAnimFolder");
	h_box_container4->add_child(button_select_anim_folder);
	button_select_anim_folder->set_custom_minimum_size(Vector2(200, 0));
	button_select_anim_folder->set_text("Select Animation folder");
//================== line_edit_select_anim_folder ==================
	line_edit_select_anim_folder->set_name("LineEditSelectAnimFolder");
	h_box_container4->add_child(line_edit_select_anim_folder);
	line_edit_select_anim_folder->set_custom_minimum_size(Vector2(200, 0));
//================== h_box_container6 ==================
	h_box_container6->set_name("HBoxContainer6");
	v_box_container->add_child(h_box_container6);
//================== button_export_animation ==================
	button_export_animation->set_name("ButtonExportAnimation");
	h_box_container6->add_child(button_export_animation);
	button_export_animation->set_custom_minimum_size(Vector2(200, 0));
	button_export_animation->set_text("Export Animation");
//================== line_edit_export_animation ==================
	line_edit_export_animation->set_name("LineEditExportAnimation");
	h_box_container6->add_child(line_edit_export_animation);
	line_edit_export_animation->set_custom_minimum_size(Vector2(200, 0));
	line_edit_export_animation->set_text("");
//================== h_box_container7 ==================
	h_box_container7->set_name("HBoxContainer7");
	v_box_container->add_child(h_box_container7);
//================== button_generate ==================
	button_generate->set_name("ButtonGenerate");
	h_box_container7->add_child(button_generate);
	button_generate->set_custom_minimum_size(Vector2(200, 0));
	button_generate->set_text("Generate");
//================== button_generate_animation ==================
	button_generate_animation->set_name("ButtonGenerateAnimation");
	h_box_container7->add_child(button_generate_animation);
	button_generate_animation->set_custom_minimum_size(Vector2(200, 0));
	button_generate_animation->set_text("Generate Animation");
//================== h_box_container8 ==================
	h_box_container8->set_name("HBoxContainer8");
	v_box_container->add_child(h_box_container8);
//================== button_select2dtexture ==================
	button_select2dtexture->set_name("ButtonSelect2dtexture");
	h_box_container8->add_child(button_select2dtexture);
	button_select2dtexture->set_custom_minimum_size(Vector2(200, 0));
	button_select2dtexture->set_text("Select 2d texture");
//================== line_edit_select2dtexture ==================
	line_edit_select2dtexture->set_name("LineEditSelect2dtexture");
	h_box_container8->add_child(line_edit_select2dtexture);
	line_edit_select2dtexture->set_custom_minimum_size(Vector2(200, 0));
	line_edit_select2dtexture->set_editable(false);
//================== h_box_container10 ==================
	h_box_container10->set_name("HBoxContainer10");
	v_box_container->add_child(h_box_container10);
//================== button_select_skeleton2d ==================
	button_select_skeleton2d->set_name("ButtonSelectSkeleton2d");
	h_box_container10->add_child(button_select_skeleton2d);
	button_select_skeleton2d->set_custom_minimum_size(Vector2(200, 0));
	button_select_skeleton2d->set_text("Select Skeleton file");
//================== line_edit_select_skeleton2d ==================
	line_edit_select_skeleton2d->set_name("LineEditSelectSkeleton2d");
	h_box_container10->add_child(line_edit_select_skeleton2d);
	line_edit_select_skeleton2d->set_custom_minimum_size(Vector2(200, 0));
	line_edit_select_skeleton2d->set_editable(false);
//================== h_box_container9 ==================
	h_box_container9->set_name("HBoxContainer9");
	v_box_container->add_child(h_box_container9);
//================== button_generate2d ==================
	button_generate2d->set_name("ButtonGenerate2d");
	h_box_container9->add_child(button_generate2d);
	button_generate2d->set_custom_minimum_size(Vector2(200, 0));
	button_generate2d->set_text("Generate 2d");
//================== button_save_viewport ==================
	button_save_viewport->set_name("ButtonSaveViewport");
	h_box_container9->add_child(button_save_viewport);
	button_save_viewport->set_custom_minimum_size(Vector2(200, 0));
	button_save_viewport->set_text("Save Viewport");






#endif