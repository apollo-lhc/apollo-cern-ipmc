#!/usr/bin/env python
# encoding: utf-8
from lxml import html
from colorama import init

import shutil
import os
import zipfile
import requests
import getpass
import sys

#Initialization
init()

USERNAME = ''
PASSWORD = ''
    
class CERNSSORequest:
    
    def __init__(self, login_url, username, passwd):
        self.session = requests.Session()
        
        #Get login form 
        result = self.session.get(login_url)
        tree = html.fromstring(result.text)
        
        #Fill the form        
        if len(tree.xpath("//input[@name='__VIEWSTATEGENERATOR']/@value")) == 0:
            __VIEWSTATEGENERATOR = ''
        else:
            __VIEWSTATEGENERATOR = tree.xpath("//input[@name='__VIEWSTATEGENERATOR']/@value")[0]
        
        if len(tree.xpath("//input[@name='__db']/@value")) == 0:
            __db = ''
        else:
            __db = tree.xpath("//input[@name='__db']/@value")[0]
        
        if len(tree.xpath("//input[@name='__LASTFOCUS']/@value")) == 0:
            __LASTFOCUS = ''
        else:
            __LASTFOCUS = tree.xpath("//input[@name='__LASTFOCUS']/@value")[0]
        
        if len(tree.xpath("//input[@name='__EVENTTARGET']/@value")) == 0:
            __EVENTTARGET = ''
        else:
            __EVENTTARGET = tree.xpath("//input[@name='__EVENTTARGET']/@value")[0]
        
        if len(tree.xpath("//input[@name='__EVENTARGUMENT']/@value")) == 0:
            __EVENTARGUMENT = ''
        else:
            __EVENTARGUMENT = tree.xpath("//input[@name='__EVENTARGUMENT']/@value")[0]
            
        if len(tree.xpath("//input[@name='__VIEWSTATE']/@value")) == 0:
            __VIEWSTATE = ''
        else:
            __VIEWSTATE = tree.xpath("//input[@name='__VIEWSTATE']/@value")[0]
    
        #Create payload (fill login FORM)
        payload = {
            "__LASTFOCUS": __LASTFOCUS,
            "__EVENTTARGET": __EVENTTARGET, 
            "__EVENTARGUMENT": __EVENTARGUMENT, 
            "__VIEWSTATE": __VIEWSTATE, 
            "__VIEWSTATEGENERATOR": __VIEWSTATEGENERATOR, 
            "__db":__db,
            "ctl00$ctl00$NICEMasterPageBodyContent$SiteContentPlaceholder$txtFormsLogin": username, 
            "ctl00$ctl00$NICEMasterPageBodyContent$SiteContentPlaceholder$txtFormsPassword": passwd, 
            "ctl00$ctl00$NICEMasterPageBodyContent$SiteContentPlaceholder$btnFormsLogin": 'Sign in', 
            "ctl00$ctl00$NICEMasterPageBodyContent$SiteContentPlaceholder$drpFederation": '-'
        }
        
        req = requests.Request('POST',result.url, data = payload)
        prepared = req.prepare()
        result = self.session.send(prepared, allow_redirects=False)
        
        if result.status_code == 302: # expected here
            jar = result.cookies
            redirect_URL2 = result.headers['Location']
            result = requests.get(redirect_URL2, cookies=jar)
        else:
            raise ValueError("LoginError: wrong ursername / password")
            
        tree = html.fromstring(result.text)
        URLFormTarget = tree.xpath("//form[@name='hiddenform']/@action")[0]
        wa = tree.xpath("//input[@name='wa']/@value")[0]
        wresult = tree.xpath("//input[@name='wresult']/@value")[0]
        
        payload = {
            "wa": wa,
            "wresult": wresult
        }
        
        req = requests.Request('POST',URLFormTarget, data = payload)
        prepared = req.prepare()
        result = self.session.send(prepared)
        
    def get(self, url):
        return requests.get(url, cookies=self.session.cookies)
        
    def post(self, url, data, files = None):
        if files != None:
            return requests.post(url, cookies=self.session.cookies, files=files, data=data)
            
        else:
            return requests.post(url, cookies=self.session.cookies, data=data)
    
    def close(self):
        self.session.close()
        
    def post_stream(self, url, data, files = None):
        strVar = ''
        
        if files != None:
            r = requests.post(url, cookies=self.session.cookies, files=files, data=data, stream=True)
            
            strVar = ''
            countLoadPoints = 0
            
            for chunk in r.iter_content(chunk_size=1): 
                if chunk: # filter out keep-alive new chunks
                
                    countLoadPoints = countLoadPoints + 1
                    if countLoadPoints > 10:
                        sys.stdout.write('\r                   \rLoading ')
                        sys.stdout.flush()
                        countLoadPoints = 0
                        
                    sys.stdout.write('.')
                    sys.stdout.flush()
                    
                    strVar = strVar + chunk.decode("utf-8")
                    split = strVar.split('\n', 1)
                    if len(split) > 1:
                        strVar = split[1]
                        if split[0].find("<a href='"):
                            sys.stdout.write('\r                   \r')
                            sys.stdout.flush()
                            countLoadPoints = 0
                            print(split[0].split(",")[0].replace('\n','').replace('\r','').replace("$",""))
                            up = split[0][split[0].find("<a href='")+9:]
                            link = 'https://cern-ipmc.web.cern.ch'+ up[:up.find("'")]
                        else:
                            sys.stdout.write('\r                   \r')
                            sys.stdout.flush()
                            countLoadPoints = 0
                            print(split[0].replace('\n','').replace('\r','').replace("$",""))

        else:
            r = requests.post(url, cookies=self.session.cookies, data=data, stream=True)
            
            strVar = ''
            countLoadPoints = 0
            
            for chunk in r.iter_content(chunk_size=1): 
                if chunk: # filter out keep-alive new chunks
                
                    countLoadPoints = countLoadPoints + 1
                    if countLoadPoints > 10:
                        sys.stdout.write('\r                   \rLoading ')
                        sys.stdout.flush()
                        countLoadPoints = 0
                        
                    sys.stdout.write('.')
                    sys.stdout.flush()
                    
                    strVar = strVar + chunk
                    split = strVar.split('\n', 1)
                    if len(split) > 1:
                        strVar = split[1]
                        if split[0].find("<a href='"):
                            sys.stdout.write('\r                   \r')
                            sys.stdout.flush()
                            countLoadPoints = 0
                            print(split[0].split(",")[0].replace('\n','').replace('\r','').replace("$",""))
                            up = split[0][split[0].find("<a href='")+9:]
                            link = 'https://cern-ipmc.web.cern.ch'+ up[:up.find("'")]
                        else:
                            sys.stdout.write('\r                   \r')
                            sys.stdout.flush()
                            countLoadPoints = 0
                            print(split[0].replace('\n','').replace('\r','').replace("$",""))
        
        sys.stdout.write('\r                   \r')
        print(strVar.replace("$",""))
        print("\033[0m")
        
        if 'downloadbin/' in link:
            print("\033[1m--> Binaries: {}\033[0m".format(link))
            return link
        else:
            print("\033[31;1m--> Compilation error \033[0m")
            return None
        
        
def zipdir(path, ziph):
        # ziph is zipfile handle
        for root, dirs, files in os.walk(path):
            for file in files:
                ziph.write(os.path.join(root, file), file)
                
def main(USERNAME, PASSWORD):

    LOGIN_URL = 'https://cern-ipmc.web.cern.ch/connect'
    URL = 'https://cern-ipmc.web.cern.ch/compileProcess'
    if sys.version_info[0] < 2 or sys.version_info[0] >3 or sys.version_info[0] == 2 and sys.version_info[1] < 4:
        print ("\n\tERROR: Python 2.xx (2.4 or higher) is required to run this program.\n\tYou can download Python from http://www.python.org") 
        sys.exit(-1)
    
    if USERNAME == '':
        if sys.version_info[0] >=3:
            USERNAME = input("CERN Username: ") 
        else:
            USERNAME = raw_input("CERN Username: ")
    else:
        print('CERN Username: {}'.format(USERNAME))
        
    if PASSWORD == '':
        PASSWORD = getpass.getpass(prompt='CERN Password: ')

    try:
        CERNSSORequestInstance = CERNSSORequest(LOGIN_URL, USERNAME, PASSWORD)
    except ValueError as e:
        print(e.message)
        exit(-1)
        
    if os.path.exists('./.tmp') and os.path.isdir('./.tmp'):
        shutil.rmtree('./.tmp')
    
    os.mkdir('./.tmp')
    
    ipmcConfigFile = ''
    
    if os.path.exists('./ipmc-config') and os.path.isdir('./ipmc-config'):    
        zipf = zipfile.ZipFile('./.tmp/ipmc-config.zip', 'w', zipfile.ZIP_DEFLATED)
        zipdir('./ipmc-config', zipf)
        zipf.close()
        
        ipmcConfigFile = './.tmp/ipmc-config.zip'
    else:
        ipmcConfigFile = './config.xml'
        
    zipf = zipfile.ZipFile('./.tmp/ipmc-sensors.zip', 'w', zipfile.ZIP_DEFLATED)
    zipdir('./ipmc-sensors', zipf)
    zipf.close()
    zipf = zipfile.ZipFile('./.tmp/ipmc-user.zip', 'w', zipfile.ZIP_DEFLATED)
    zipdir('./ipmc-user', zipf)
    zipf.close()
    
    ipmcConfigFile_ptr = open(ipmcConfigFile, 'rb')
    ipmcSensorZipFile_ptr = open('./.tmp/ipmc-sensors.zip', 'rb')
    ipmcUserZipFile_ptr = open('./.tmp/ipmc-user.zip', 'rb')
    
    files = {'ipmcConfigZipFile': ipmcConfigFile_ptr, 'ipmcSensorZipFile': ipmcSensorZipFile_ptr, 'ipmcUserZipFile': ipmcUserZipFile_ptr}
    values = {'method': 'advanced', 'dest': 'cliTool'}
    link = CERNSSORequestInstance.post_stream(URL, data = values, files = files)    
    
    if link != None:        
        
        print("\033[1m--> Download and extract hpm.1 image \033[0m")
        outzip = open('./.tmp/ipmc-binaries.zip', 'wb+')
        result = CERNSSORequestInstance.get(link)
        outzip.write(result.content)
        outzip.close()
        
        zip_ref = zipfile.ZipFile('./.tmp/ipmc-binaries.zip', 'r')
        zip_ref.extractall('./.tmp/ipmc-binaries')
        zip_ref.close()
        
        sourcefiles = './.tmp/ipmc-binaries'
        destinationpath = './'
        for file in os.listdir(sourcefiles):
            if file.endswith('.img') or file.endswith('.ihx'):
                if os.path.exists(os.path.join(destinationpath,file)):
                    os.remove(os.path.join(destinationpath,file))
                os.rename(os.path.join(sourcefiles,file), os.path.join(destinationpath,file))
        
        CERNSSORequestInstance.close()   

    ipmcConfigFile_ptr.close()
    ipmcSensorZipFile_ptr.close()
    ipmcUserZipFile_ptr.close()
    
    if os.path.exists('./.tmp') and os.path.isdir('./.tmp'):
        shutil.rmtree('./.tmp')
    
if __name__ == '__main__':
    main(USERNAME, PASSWORD)