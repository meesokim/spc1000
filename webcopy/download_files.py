import sys, os, requests, time, threading, urllib3, zipfile, random
from urllib.parse import urlparse
urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)
from concurrent.futures import ThreadPoolExecutor

def download(url, fullpath):
    global filename, ran
    # print(fullpath, 'downloading ...')
    try:
        header = {"User-Agent":'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36'}
        response = requests.get(url, headers=header)
        rcode = response.status_code
        if rcode == 200 and len(response.content):
            open(fullpath,'wb').write(response.content)
            print(fullpath, ' ... completed')
        else:
            open(f'{filename}.err','a').write(f'{rcode},{url}\n')
            print(url, 'error', rcode)
    except:
        import traceback
        # traceback.print_exc()
        print(traceback.format_exc())
        # time.sleep(5)
        print(url, 'error')
        download(url, fullpath)
    # if 'ran' in globals():
    #     time.sleep(random.randint(1,ran))

if __name__=='__main__':
    subpath = ''
    zipcheck = False
    if len(sys.argv) > 1:
        filename = sys.argv[1]
        if len(sys.argv) > 2:
            subpath = sys.argv[2]
    else:
        print(sys.argv[0], 'usage: filename')
        os.exit()
    urls = []
    paths = []
    if os.path.exists(filename):
        files = open(filename,'r').read().split('\n')
        filename404 = f'{filename}.404'
        filenameerr = f'{filename}.err'
        if os.path.exists(filename404):
            f404 = open(filename404,'r').read().split('\n')
            files = list(set(files)-set(f404))
        if os.path.exists(filenameerr):
            ferr = open(filenameerr,'r').read().split('\n')
            files = list(set(files)-set([line[4:] for line in ferr]))
        for url in files:
            if '?' in url:
                continue
            url = url.replace('/./','/')
            o = urlparse(url)
            fullpath = f'{os.environ['HOME']}/{o.hostname}{requests.utils.unquote(o.path)}'
            if len(subpath) and not subpath in fullpath:
                continue
            path = '/'.join(fullpath.split('/')[:-1])
            if os.path.exists(fullpath) and os.stat(fullpath).st_size > 0:
                ret = None
                ftext = os.path.splitext(fullpath)
                if zipcheck and ftext[1].lower() == '.zip':
                    import magic
                    datatype = magic.from_file(fullpath)
                    if not 'zip' in datatype.lower():
                        ret = False
                        print(fullpath, datatype, 'download again ...')
                    # print(fullpath, 'zipfile check')
                    # try:
                    #     the_zip_file = zipfile.ZipFile(fullpath)
                    #     ret = the_zip_file.testzip()
                    # except zipfile.BadZipfile as ex:
                    #     # print(fullpath, 'is not a zipfile?')
                    #     # print(datatype)
                    # except:
                    #     import traceback
                    #     traceback.print_exc()
                    #     ret = False
                if ret is None:
                    # print(fullpath, 'already exists')
                    continue
            # else:
            #     print(fullpath.split('/')[-1], ' not exist ...', end='\r')
            if not os.path.exists(path):
                os.makedirs(path)
            urls.append(url)
            paths.append(fullpath)
        print(f'Total downloading files:{len(urls)}')
        with ThreadPoolExecutor(max_workers=5) as executor:
            results = list(executor.map(download, urls, paths))
            # task = threading.Thread(target=download, args=(url, fullpath))
            # task.start()
            # time.sleep(2)
    else:
        print(filename, 'not exists')
