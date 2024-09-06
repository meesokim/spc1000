import sys, os, requests, time
from urllib.parse import urlparse

if __name__=='__main__':
    if len(sys.argv) > 1:
        filename = sys.argv[1]
    else:
        print(sys.argv[0], 'usage: filename')
        os.exit()
    if os.path.exists(filename):
        files = open(filename,'r').read().split('\n')
        for url in files:
            o = urlparse(url)
            fullpath = f'{os.environ['HOME']}/{o.hostname}{requests.utils.unquote(o.path)}'
            path = '/'.join(fullpath.split('/')[:-1])
            if os.path.exists(fullpath):
                print(fullpath, 'already exists')
                continue
            print(fullpath, 'downloading... ')
            if not os.path.exists(path):
                os.makedirs(path)
            response = requests.get(url)
            if response.status_code == 200:
                open(fullpath,'wb').write(response.content)
                time.sleep(5)
            else:
                print(url, 'error')
    else:
        print(filename, 'not exists')
