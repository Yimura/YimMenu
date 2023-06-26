import json

natives = json.load(open("natives.json"))
out_file : str = ""

def sanitize_param(name):
    if name in ["repeat", "end"]:
        return "_"+name
    else:
        return name

def make_param_listing(params):
    pms = ""
    for param in params:
        pms += sanitize_param(param["name"])
        pms += ","
    pms = pms.rstrip(",")
    return pms

def is_string(type: str):
    return type.find("char*") != -1

def is_pointer(type: str):
    """also returns true for string"""
    return type.find('*') != -1


def write_native(name, hash, params, return_type):
    global out_file

    out_file += f"{name}=function({make_param_listing(params)})"

    invoke_type = "invoke_int"

    if (return_type == "void"):
        invoke_type = "invoke_void"
    elif (return_type == "float"):
        invoke_type = "invoke_float"
    elif (return_type == "BOOL"):
        invoke_type = "invoke_bool"
    elif (return_type == "const char*"):
        invoke_type = "invoke_str"
    elif (return_type == "Vector3"):
        invoke_type = "invoke_vec3"
    elif (return_type.endswith("*")):
        invoke_type = "invoke_ptr"

    out_file += f"return _natives.{invoke_type}({hash},"
    for param in params:
        if param['type'] == "float":
            out_file += "0xDEADDEADDEADDEAD,"
        out_file += f"{sanitize_param(param['name'])},"
    out_file = out_file.removesuffix(",")
    out_file += ");end,\n"

def write_namespace(name, data):
    global out_file

    out_file += f"{name} = {{\n"

    for (hash, more) in data.items():
        write_native(more["name"], hash, more["params"], more["return_type"])
    
    out_file += "};\n"

def write_file():
    for (namespace, data) in natives.items():
        write_namespace(namespace, data)

def convert_and_write_cpp_file():
    global out_file

    cpp_data = "#pragma once\n// clang-format off\n// Generated by natives_gen.py. DO NOT EDIT\nchar natives_data[] = \n"

    lines = out_file.rstrip('\n').splitlines()
    for line in lines:
        cpp_data += f"\"{line}\\n\"\\\n"
    
    cpp_data = cpp_data.rstrip('\n\\')
    cpp_data += ";\n// clang-format on\n"
    cpp_data += "int natives_size = sizeof(natives_data)-1;"
    open("natives_data.cpp", "w+").write(cpp_data)

def write_lua_file():
    open("natives.lua", "w+").write(out_file)

if __name__ == "__main__":
    write_file()
    convert_and_write_cpp_file()