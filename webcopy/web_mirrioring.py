import sys
import requests
from bs4 import BeautifulSoup

def get_links(url, link=''):
    if url[-1] != '/':
        url = url + '/' + link
    else:
        url = url + link
    print(url)
    fileurls = []
    response = requests.get(url)
    if response.status_code == 200:
        html = response.text
        soup = BeautifulSoup(html, 'html.parser')    
        for tag in soup.find_all('a', href=True):
            link = tag['href']
            if link[0] >= '0' and link[0] != '?' and link[0] != '/' and link[:4] != 'http':
                if link[-1] == '/':
                    # suburls.append(url + '/' + link)
                    fileurls.extend(get_links(url, link))
                else:
                    fileurls.append(url+link)
            # else:
            #     print(f'{link} removed')
    if len(fileurls) > 0:
        print('\n'.join(fileurls))
    return fileurls

if __name__=='__main__':
    args = sys.argv
    url = 'http://cpmarchives.classiccmp.org/cpm/mirrors/oak.oakland.edu/pub'
    file = 'files.txt'
    if len(args) > 1:
        url = args[1]
    if len(args) > 2:
        file = args[2]
    fileurls = get_links(url)
    open(file, 'w').write('\n'.join(fileurls))
