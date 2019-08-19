import sys
import os
from string import Template

ip_addr_map = {}
ip_addr_map[0] = "192.168.1.34"
ip_addr_map[3] = "192.168.20.72"
ip_addr_map[5] = "192.168.20.56"
ip_addr_map[7] = "192.168.20.69"

mac_addr_map = {}
mac_addr_map[0] = "0A:0A:0A:0A:0A:CC"
mac_addr_map[3] = "00:50:51:FF:00:03"
mac_addr_map[5] = "00:50:51:FF:00:05"
mac_addr_map[7] = "00:50:51:FF:00:07"

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

    ipmc_id = int(sys.argv[1])

    # get scripts directory path
    scripts_dir = os.path.dirname(os.path.realpath(__file__))

    # find the proper template file to construct the config.xml
    templates_dir = os.path.join(scripts_dir, "../templates")
    if ipmc_id == 0:
        template_file = os.path.join(templates_dir, "config_devkit.tpl")
    else:
        template_file = os.path.join(templates_dir, "config_sm.tpl")

    target_file = os.path.join(scripts_dir, "../config.xml")

    with open(target_file, 'w') as f:
        f.write(fill_template(template_file, ipmc_id))

if __name__ == "__main__":
    main()
