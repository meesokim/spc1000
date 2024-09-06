import requests
fileurls = []
url = 'https://www.msxarchive.nl/pub/msx'
res = requests.get(url + '/allfiles.txt')
if res.status_code == 200:
    files = res.text
    for line in files.split('\n'):
        if len(line) == 0 or line[:5] == 'total':
            continue
        elif line[-1] == ':':
            path = line[1:-1]
        else:
            items = line.split(' ')
            if items[0][0] == 'd' or items[0][0] == 'l' or items[-1][-4:] == '.bak':
                continue
            fileurl = url + requests.utils.quote(path + '/' + items[-1])
            fileurls.append(fileurl)
    print('\n'.join(fileurls))
