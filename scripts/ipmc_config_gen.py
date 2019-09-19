import sys
import os
from string import Template

ip_addr_map = {}
ip_addr_map[0] = "192.168.1.34"
ip_addr_map[2] = "192.168.1.67"
ip_addr_map[3] = "192.168.20.72"
ip_addr_map[5] = "192.168.20.56"
ip_addr_map[6] = "192.168.20.57"
ip_addr_map[7] = "192.168.20.69"
ip_addr_map[8] = "192.168.20.60"
ip_addr_map[9] = "192.168.20.62"
ip_addr_map[10] = "192.168.20.63"

mac_addr_map = {}
mac_addr_map[0] = "0A:0A:0A:0A:0A:CC"
mac_addr_map[2] = "00:50:51:FF:00:02"
mac_addr_map[3] = "00:50:51:FF:00:03"
mac_addr_map[5] = "00:50:51:FF:00:05"
mac_addr_map[6] = "00:50:51:FF:00:06"
mac_addr_map[7] = "00:50:51:FF:00:07"
mac_addr_map[8] = "00:50:51:FF:00:08"
mac_addr_map[9] = "00:50:51:FF:00:09"
mac_addr_map[10] = "00:50:51:FF:00:10"

def fill_template(template_file, ipmc_id):

    params = {}
    try:
        params['ip_address'] = ip_addr_map[ipmc_id] 
        params['mac_address'] = mac_addr_map[ipmc_id]
    except:
        print("Wrong Service Module ID")
        exit(1)
        
    template = Template(open(template_file, 'r').read())
    return template.substitute(params)
    
def main():

    if (len(sys.argv) != 3):
        print("Usage: {} <SM #> <SDI or SOL>".format(sys.argv[0]))
        exit(1)

    ipmc_id = int(sys.argv[1])

    serial_if = sys.argv[2].lower()

    # get scripts directory path
    scripts_dir = os.path.dirname(os.path.realpath(__file__))

    # find the proper template file to construct the config.xml
    templates_dir = os.path.join(scripts_dir, "../templates")
    if ipmc_id == 0:
        template_file = os.path.join(templates_dir, "config_devkit.tpl")
    else:
        if serial_if == "sdi":
            template_file = os.path.join(templates_dir, "config_sm_sdi.tpl")
        elif(serial_if == "sol"):
            template_file = os.path.join(templates_dir, "config_sm_sol.tpl")
        else:
            print("Unknown serial mode")
            exit(1)

    target_file = os.path.join(scripts_dir, "../config.xml")

    with open(target_file, 'w') as f:
        f.write(fill_template(template_file, ipmc_id))

if __name__ == "__main__":
    main()
