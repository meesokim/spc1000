import sys, os, requests, time, threading, urllib3, zipfile
from urllib.parse import urlparse
urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)

def download(url, fullpath):
    global filename
    response = requests.get(url, verify=False)
    if response.status_code == 200:
        open(fullpath,'wb').write(response.content)
    if response.status_code == 404:
        open(f'{filename}.404','a').write(url+'\n')
        print(url, 'error', response.status_code)

if __name__=='__main__':
    subpath = ''
    if len(sys.argv) > 1:
        filename = sys.argv[1]
        if len(sys.argv) > 2:
            subpath = sys.argv[2]
    else:
        print(sys.argv[0], 'usage: filename')
        os.exit()
    if os.path.exists(filename):
        files = open(filename,'r').read().split('\n')
        filename404 = f'{filename}.404'
        if os.path.exists(filename404):
            f404 = open(filename404,'r').read().split('\n')
            files = list(set(files)-set(f404))
        for url in files:
            if '?' in url:
                continue
            o = urlparse(url)
            fullpath = f'{os.environ['HOME']}/{o.hostname}{requests.utils.unquote(o.path)}'
            if len(subpath) and not subpath in fullpath:
                continue
            path = '/'.join(fullpath.split('/')[:-1])
            if os.path.exists(fullpath) and os.stat(fullpath).st_size > 0:
                ret = None
                ftext = os.path.splitext(fullpath)
                if ftext[1].lower() == '.zip':
                    # print(fullpath, 'zipfile check')
                    try:
                        the_zip_file = zipfile.ZipFile(fullpath)
                        ret = the_zip_file.testzip()
                    except zipfile.BadZipfile as ex:
                        print(fullpath, 'is not a zipfile?')
                        import magic
                        print(magic.from_file(fullpath))
                    except:
                        ret = False
                if ret is None:
                    # print(fullpath, 'already exists')
                    continue
            print(fullpath, 'downloading ...')
            if not os.path.exists(path):
                os.makedirs(path)
            task = threading.Thread(target=download, args=(url, fullpath))
            task.start()
            time.sleep(2)
    else:
        print(filename, 'not exists')
