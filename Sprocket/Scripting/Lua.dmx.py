from Datamatic.Plugins import Plugin, compmethod

class Lua(Plugin):
    
    @compmethod
    def Sig(comp, flags):
        return ", ".join(attr['Name'] for attr in comp['Attributes'] if attr["Flags"]["SCRIPTABLE"])

    @compmethod
    def Impl(comp, flags):
        out = ""
        num_attrs = 0
        constructor_sig = []
        for attr in comp["Attributes"]:
            if not attr["Flags"]["SCRIPTABLE"]:
                continue
            if attr["Type"]  == "glm::vec4":
                constructor_sig.append(f"Vec4(x{num_attrs}, x{num_attrs+1}, x{num_attrs+2}, x{num_attrs+3})")
                num_attrs += 4
            elif attr["Type"] == "glm::vec3":
                constructor_sig.append(f"Vec3(x{num_attrs}, x{num_attrs+1}, x{num_attrs+2})")
                num_attrs += 3
            elif attr["Type"] == "glm::vec2":
                constructor_sig.append(f"Vec3(x{num_attrs}, x{num_attrs+1})")
                num_attrs += 2
            else:
                constructor_sig.append(f"x{num_attrs}")
                num_attrs += 1

        name = comp["Name"]
        if not comp["Flags"]["SCRIPTABLE"]:
            return out

        out += f'function Get{name}(entity)\n'
        pack = ", ".join([f'x{i}' for i in range(num_attrs)])
        out += "    " + pack + f" = Lua_Get{name}(entity)\n"
        out += f'    return {name}({", ".join(constructor_sig)})\n'
        out += "end\n\n"

        out += f'function Set{name}(entity, c)\n'
        out += f'    Lua_Set{name}(entity, '
        args = []
        for attr in comp["Attributes"]:
            n = attr["Name"]
            if not attr["Flags"]["SCRIPTABLE"]:
                continue
            if attr["Type"] == "glm::vec3":
                args.extend([f'c.{n}.x', f'c.{n}.y', f'c.{n}.z'])
            else:
                args.append(f'c.{attr["Name"]}')
        out += ", ".join(args)
        out += ')\n'
        out += "end\n\n"

        out += f'function Add{name}(entity, c)\n'
        out += f'    Lua_Add{name}(entity, '
        args = []
        for attr in comp["Attributes"]:
            n = attr["Name"]
            if not attr["Flags"]["SCRIPTABLE"]:
                continue
            if attr["Type"] == "glm::vec3":
                args.extend([f'c.{n}.x', f'c.{n}.y', f'c.{n}.z'])
            else:
                args.append(f'c.{attr["Name"]}')
        out += ", ".join(args)
        out += ')\n'
        out += "end\n"
        return out