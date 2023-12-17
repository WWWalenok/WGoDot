import bpy
import json
from pathlib import Path

from mathutils import * 

import os
clear = lambda: os.system('cls')
clear()

output_path = "E:\\Repos\\WGdotTest\\" # change

output_name = "_armatures" # change

input_path = "E:\\Repos\\WGdotTest\\skelet.json"

json_input_path = Path(str(input_path))

json_output_path = Path(str(output_path))

id = 0

def export_skeleton():
    for ob in bpy.data.objects:
        print(ob.type)
        Armatures = {}
        
        if ob.type == 'ARMATURE':
            armature = ob
            bones = []
            bid = 0
            for bone in armature.pose.bones:
                print(bone.name)
                bm = bone.matrix
                bn = {}
                
                bn["name"] = bone.name
    #            bn["id"] = bid
                bn["parent"] = -1
                pbid = 0
                if bone.parent is not None:
                    for bone_2 in armature.pose.bones:
                        
                        if bone.parent == bone_2:
                            bn["parent"] = pbid
                            break
                        pbid += 1
                
                bid += 1
                if bn["parent"] != -1:
                    tr = bone.parent.bone.matrix_local.inverted() * bone.bone.matrix_local
                    bn["transform_mat"] = []
                    for i in range(4):
                        t = []
                        for j in range(4):
                            t.append(1 if i == j else 0)
                        bn["transform_mat"].append(t)
                    
                    t = bone.head - bone.parent.tail
                    bn["transform_mat"][3][0] = t.x
                    bn["transform_mat"][3][1] = t.y
                    bn["transform_mat"][3][2] = t.z
                else:
                    bn["transform_mat"] = []
                    for i in range(4):
                        t = []
                        for j in range(4):
                            t.append(1 if i == j else 0)
                        bn["transform_mat"].append(t)
                    

                bones.append(bn)
                
            with open(output_path / Path(output_name + "." + str(id) + "_" + ob.name  + ".bone.json"), "w") as out_file:
                out_file.write(json.dumps(bones, indent=1))


def import_skeleton():
    selected = []
    try:
        with open(json_input_path, "r") as json_file:
            for i in bpy.context.scene.objects:
                if i.select_get():
                    selected.append(i)
                i.select_set(False)
                

            try:
                armature_o = bpy.context.scene.objects["genned_arma_o"]
                armature = bpy.data.armatures["genned_arma"]
            except:
                armature = bpy.ops.object.armature_add()
                armature = bpy.data.armatures[-1]
                armature_o = bpy.context.scene.objects[-1]
                print(armature)
                armature.name = "genned_arma"
                armature_o.name = "genned_arma_o"

            print(bpy.context.scene.objects)
            for i in bpy.context.scene.objects:
                print(i)

            edit_bones = armature_o.data.edit_bones
            json_data = json.load(json_file)
            print()
            bpy.context.view_layer.objects.active = armature_o
            armature_o.select_set(False)
            bpy.ops.object.mode_set(mode='EDIT', toggle=False)
            bone_map = []
            for bone in edit_bones:
                edit_bones.remove(bone)


            def mult_trans(a, b):

                rows_a, cols_a = len(a), len(a[0])
                rows_b, cols_b = len(b), len(b[0])

                if cols_a != rows_b:
                    print(a)
                    print(b)
                    raise ValueError("Cannot multiply matrices:  incompatible dimensions.")

                ret = [[0 for _ in range(cols_b)] for _ in range(rows_a)]

                for i in range(rows_a):
                    for j in range(cols_b):
                        for k in range(cols_a):
                            ret[i][j] += a[i][k] * b[k][j]

                return ret
            
            base_node = None
            
            def rec_applay(i, trans):
                bn = json_data[i]
                tm = bn["transform_mat"]
                c_trans_1 = [
                    [tm[0][0], tm[0][1], tm[0][2]],
                    [tm[1][0], tm[1][1], tm[1][2]],
                    [tm[2][0], tm[2][1], tm[2][2]]
                ]
                c_trans_2 = [
                    [tm[0][0], tm[1][0], tm[2][0]],
                    [tm[0][1], tm[1][1], tm[2][1]],
                    [tm[0][2], tm[1][2], tm[2][2]]
                ]

                n_trans = mult_trans(trans, c_trans_1)

                co = [[tm[3][0]], [tm[3][1]], [tm[3][2]]]

                r_co = mult_trans(trans, co)

                co = [r_co[0][0], r_co[2][0], r_co[1][0]]

                print(bn['name'])
                sel = edit_bones.new(bn['name'])
                bone_map.append(sel)

                if bn['parent'] != -1:
                    sel.parent = bone_map[bn['parent']]
                    co[0] = co[0] + sel.parent.head.x
                    co[1] = co[1] + sel.parent.head.y
                    co[2] = co[2] + sel.parent.head.z
                    sel.tail = (co[0], co[1], co[2] + 0.1)
                    sel.head = (co[0], co[1], co[2])
                    
                else:
                    sel.parent = base_node
                    sel.tail = (co[0], co[1], co[2] + 0.1)
                    sel.head = (co[0], co[1], co[2])


                for j in range(i, len(json_data)):
                    if json_data[j]['parent'] == i:
                        rec_applay(j, n_trans)



            b_trans = [
                [1, 0, 0],
                [0, 1, 0],
                [0, 0, 1]
            ]

            b_offset = [0, 0, 0]

            rec_applay(0, b_trans)

                    
                
    except Exception as e:
        print(e)
        do_nothing = ""
    bpy.ops.object.mode_set(mode='OBJECT', toggle=False)
    for select in selected:
        select.select_set(True)

    
import_skeleton()
