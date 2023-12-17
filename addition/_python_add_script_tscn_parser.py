LayoutPreset = [
    "Control::PRESET_TOP_LEFT",
    "Control::PRESET_TOP_RIGHT",
    "Control::PRESET_BOTTOM_LEFT",
    "Control::PRESET_BOTTOM_RIGHT",
    "Control::PRESET_CENTER_LEFT",
    "Control::PRESET_CENTER_TOP",
    "Control::PRESET_CENTER_RIGHT",
    "Control::PRESET_CENTER_BOTTOM",
    "Control::PRESET_CENTER",
    "Control::PRESET_LEFT_WIDE",
    "Control::PRESET_TOP_WIDE",
    "Control::PRESET_RIGHT_WIDE",
    "Control::PRESET_BOTTOM_WIDE",
    "Control::PRESET_VCENTER_WIDE",
    "Control::PRESET_HCENTER_WIDE",
    "Control::PRESET_FULL_RECT"
]

GrowDirection = [
    "Control::GROW_DIRECTION_BEGIN",
    "Control::GROW_DIRECTION_END",
    "Control::GROW_DIRECTION_BOTH"
]

HorizontalAlignment = [
	"HORIZONTAL_ALIGNMENT_LEFT",
	"HORIZONTAL_ALIGNMENT_CENTER",
	"HORIZONTAL_ALIGNMENT_RIGHT",
	"HORIZONTAL_ALIGNMENT_FILL"
]

VerticalAlignment = [
	"VERTICAL_ALIGNMENT_TOP",
	"VERTICAL_ALIGNMENT_CENTER",
	"VERTICAL_ALIGNMENT_BOTTOM",
	"VERTICAL_ALIGNMENT_FILL",
]

TextServerOverrunBehavior = [
    "TextServer::OVERRUN_NO_TRIMMING",
    "TextServer::OVERRUN_TRIM_CHAR",
    "TextServer::OVERRUN_TRIM_WORD",
    "TextServer::OVERRUN_TRIM_ELLIPSIS",
    "TextServer::OVERRUN_TRIM_WORD_ELLIPSIS",
]

FileDialogFileMode = [
    "FileDialog::FILE_MODE_OPEN_FILE",
    "FileDialog::FILE_MODE_OPEN_FILES",
    "FileDialog::FILE_MODE_OPEN_DIR",
    "FileDialog::FILE_MODE_OPEN_ANY",
    "FileDialog::FILE_MODE_SAVE_FILE"
]

LayoutDirection = [
    "Control::LAYOUT_DIRECTION_INHERITED",
    "Control::LAYOUT_DIRECTION_LOCALE",
    "Control::LAYOUT_DIRECTION_LTR",
    "Control::LAYOUT_DIRECTION_RTL"
]

TreeSelectMode = [
    "Tree::SELECT_SINGLE",
    "Tree::SELECT_ROW",
    "Tree::SELECT_MULTI"
]
in_h = ""
in_cpp = ""
in_cpp_2 = ""
in_cpp_3 = ""
in_cpp_4 = ""

MainName = "RiggingSystemDock"

import os

def gen_name(tname):
    name = ""

    if (tname[0].isupper()):
        name += tname[0].lower()
    else:
        name += tname[0]

    for j in range(1, len(tname)):
        if (tname[j].isupper()):
            name += "_" + tname[j].lower()
        else:
            name += tname[j]

    return name

def get_val(line, val):
    ret = ""
    if(val not in line):
        return ""
    i = line.find(val)
    while line[i] != '=':
        i += 1
    while line[i] != '\"':
        i += 1
    i += 1
    while line[i] != '\"':
        ret += line[i]
        i += 1

    return ret

def get_parent(parent):
    if (parent == ""):
        return ""
    if (parent == "."):
        return "this"
    return gen_name(parent.split("/")[-1])

child_map = {}

file_name = "./addition/control.tscn"

file_out_h = "./addition/layouts/rigging_dock_hpp.h"
file_out_cpp = "./addition/layouts/rigging_dock_cpp.h"

guard = "__LAYOUT__"


with open(file_name, "r") as file:
    print(f"open {file_name}")
    line = "\n"
    name = ""
    type = ""
    parent = ""
    count = 0
    while(line != ""):
        line = file.readline()
        if("node" in line):
            count += 1
            parent = type = name = ""

            name = get_val(line, "name")
            type = get_val(line, "type")
            parent = get_val(line, "parent")

            fname = name
            name = gen_name(fname)
            prnt = get_parent(parent)

            if count == 1:
                name = "this"
                type = MainName

            in_cpp_2 += f"//================== {name} ==================\n"
            in_cpp_2 += f"\t{name}->set_name(\"{fname}\");\n"

            if count != 1:
                in_h += f"\t{type}* {name} = nullptr;\n"
                in_cpp += f"\t{name} = memnew({type});\n"
                in_cpp_2 += f"\t{prnt}->add_child({name});\n"

            child_map[name] = {'name' : name, 'type' : type, 'fname' : fname, 'prnt' : prnt}
            print(f"process ({fname}) -> {type} {name} ")

        elif("connection" in line):

            _signal = get_val(line, "signal")
            _from = get_val(line, "from")
            _to = get_val(line, "to")
            _method = get_val(line, "method")

            _from = get_parent(_from)
            _to = get_parent(_to)

            _from_el = child_map[_from]
            _to_el = child_map[_to]

            in_cpp_3 += f"""\t{_from}->connect("{_signal}", callable_mp({_to}, &{_to_el['type']}::{_method}));\n"""

            in_cpp_4 += f"""void {_method}();\n"""

        else: ## vars
            if(len(line) > 1 and line[0] != '['):
                par = ""
                val = ""
                i = 0

                while i < len(line) and line[i] != '=' and line[i] != ' ' and line[i] != '\t':
                    par += line[i]
                    i += 1

                while i < len(line) and (line[i] == '=' or line[i] == ' ' or line[i] == '\t'):
                    i += 1

                while i < len(line) and line[i] != '\n':
                    val += line[i]
                    i += 1

                not_supporded = [
                    "layout_mode",
                    "script",
                    "scroll_deadzone",
                ]

                if (par == "anchors_preset"):
                    in_cpp_2 += f"\t{name}->set_anchors_preset({LayoutPreset[int(val)]});\n"

                elif (par == "anchor_right"):
                    in_cpp_2 += f"\t{name}->set_anchor(SIDE_RIGHT, {val});\n"

                elif (par == "anchor_left"):
                    in_cpp_2 += f"\t{name}->set_anchor(SIDE_LEFT, {val});\n"

                elif (par == "anchor_bottom"):
                    in_cpp_2 += f"\t{name}->set_anchor(SIDE_BOTTOM, {val});\n"

                elif (par == "anchor_top"):
                    in_cpp_2 += f"\t{name}->set_anchor(SIDE_TOP, {val});\n"

                elif (par == "grow_horizontal"):
                    in_cpp_2 += f"\t{name}->set_h_grow_direction({GrowDirection[int(val)]});\n"

                elif (par == "grow_vertical"):
                    in_cpp_2 += f"\t{name}->set_v_grow_direction({GrowDirection[int(val)]});\n"

                elif (par == "offset_right"):
                    in_cpp_2 += f"\t{name}->set_offset(SIDE_RIGHT, {val});\n"

                elif (par == "offset_left"):
                    in_cpp_2 += f"\t{name}->set_offset(SIDE_LEFT, {val});\n"

                elif (par == "offset_bottom"):
                    in_cpp_2 += f"\t{name}->set_offset(SIDE_BOTTOM, {val});\n"

                elif (par == "offset_top"):
                    in_cpp_2 += f"\t{name}->set_offset(SIDE_TOP, {val});\n"

                elif (par == "alignment" and type == "Button"):
                    in_cpp_2 += f"\t{name}->set_text_alignment({HorizontalAlignment[int(val)]});\n"

                elif (par == "text_overrun_behavior" and type == "Button"):
                    in_cpp_2 += f"\t{name}->set_text_overrun_behavior({TextServerOverrunBehavior[int(val)]});\n"

                elif (par == "layout_direction"):
                    in_cpp_2 += f"\t{name}->set_layout_direction({LayoutDirection[int(val)]});\n"

                elif (par == "size_flags_horizontal"):
                    in_cpp_2 += f"\t{name}->set_h_size_flags({val});\n"

                elif (par == "size_flags_vertical"):
                    in_cpp_2 += f"\t{name}->set_v_size_flags({val});\n"

                elif (par == "select_mode" and type == "Tree"):
                    in_cpp_2 += f"\t{name}->set_select_mode({TreeSelectMode[int(val)]});\n"

                elif (par == "file_mode" and type == "FileDialog"):
                    in_cpp_2 += f"\t{name}->set_file_mode({FileDialogFileMode[int(val)]});\n"

                elif (par == "filters" and type == "FileDialog"):
                    n_val = val[val.find('(') + 1 : -1]
                    in_cpp_2 += f"\t{name}->set_filters({{{n_val}}});\n"

                elif (type == "PopupMenu" and par == "item_count"):
                    in_cpp_2 += f"\t{name}->set_item_count({val});\n"

                elif (type == "PopupMenu" and par.startswith("item_")):
                    id = 0
                    ids = ""
                    sub_par = ""
                    j = 5
                    while (par[j] != '/'):
                        ids += par[j]
                        j += 1
                    id = int(ids)
                    j += 1
                    while (j < len(par)):
                        sub_par += par[j]
                        j += 1
                    in_cpp_2 += f"\t{name}->set_item_{sub_par}({ids}, {val});\n"

                elif par not in not_supporded:
                    in_cpp_2 += f"\t{name}->set_{par}({val});\n"

import datetime;
with open(file_out_h, "w") as out:
    print(f"write header to {file_out_h}")
    out.write(f"#ifndef {guard}H\n#define {guard}H\n///\n/// Autogen using tscn_parser.py by WWWalenok v 0.02\n/// Autogen based on {file_name}\n/// {datetime.datetime.now()}\n///\n\n")
    out.write(in_h)
    out.write("\n\n#endif")
    


with open(file_out_cpp, "w") as out:
    print(f"write code to {file_out_cpp}")
    out.write(f"#ifndef {guard}CPP\n#define {guard}CPP\n///\n/// Autogen using tscn_parser.py by WWWalenok v 0.02\n/// Autogen based on {file_name}\n/// {datetime.datetime.now()}\n///\n\n")
    out.write(in_cpp + "\n\n" + in_cpp_2 + "\n\n" + in_cpp_3 + "\n\n" + in_cpp_4)
    out.write("\n\n#endif")
    

print("end")