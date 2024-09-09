import sys
import requests, time
from bs4 import BeautifulSoup
from requests_html import HTMLSession

import nest_asyncio
from urllib.parse import urlparse
nest_asyncio.apply()
# from requests_html import AsyncHTMLSession
from requests_html import HTML, HTMLSession, AsyncHTMLSession
gurls = []
def get_links(url, link=''):
    global gurls
    if len(link):
        if url[-4:] != 'html' and url[-3:] != 'htm' and url[-1] != '/' and '?' not in url:
            url = url + '/' + link
        else:
            url = url + link
    if url in gurls:
        return []
    print('get_links:', url)
    o = urlparse(url)
    fileurls = []
    session = HTMLSession()
    try:
        r = session.get(url, verify=False)
        r.html.arender()
        if len(o.path) > 1:
            surl = url.replace(o.path, '')
        else:
            surl = url
        print(1, surl, o.path)
        print(r.html.links)
        for link in r.html.links:
            print('link', link)
            if '../' in link:
                continue
            if (not 'http' in link) and len(link) > 1:
                if link[0] == '/':
                    link = surl + link
                else:
                    if url[-1] == '/':
                        link = url + link
                    else:
                        link = url + '/' + link
                if link[-1] == '/' or link[-4:] == 'html' or link[-3:] == 'htm':
                    gurls.append(url)
                    fileurls.extend(get_links(link))
                elif '?' not in link:
                    print(link)
                    fileurls.append(link)
            elif surl in link and '?' in link:
                fileurls.extend(get_links(link))
    except:
        import traceback
        traceback.print_exc()
        time.sleep(5)
        fileurls.extend(get_links(url))
    return fileurls
    # response = requests.get(url)
    # if response.status_code == 200:
    #     html = response.text
    #     # print(html)
    #     soup = BeautifulSoup(html, 'html.parser')
    #     for tag in soup.find_all('a', href=True):
    #         link = tag['href']
    #         if link[0] >= '0' and link[0] != '?' and link[0] != '/' and link[:4] != 'http':
    #             if link[-1] == '/':
    #                 # suburls.append(url + '/' + link)
    #                 fileurls.extend(get_links(url, link))
    #             else:
    #                 fileurls.append(url+link)
    #         # else:
    #         #     print(f'{link} removed')
    # else:
    #     print(response.status_code)

    # if len(fileurls) > 0:
    #     print('\n'.join(fileurls))
    # return fileurls

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
