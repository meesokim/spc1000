import sys
import requests, time, urllib3
from bs4 import BeautifulSoup
from requests_html import HTMLSession
urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)
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
    print(f'\033[0;32m{url}\033[0m')
    url0 = url
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
        if '.html' == url[-5:] or '.htm' == url[-4:]:
            url = '/'.join(url.split('/')[:-1])
        # print(1, surl, o.path, url)
        # print(r.html.links)
        for link in r.html.links:
            if '../' in link:
                continue
            if (not 'http' in link) and len(link) > 1:
                if link[0] == '/':
                    link = surl + link
                else:
                    # if url[-1] == '/':
                    #     link = url + link
                    # else:
                    link = url + ('/' if url[-1] != '/' else '') + link
                if link[-1] == '/' or link[-4:] == 'html' or link[-3:] == 'htm':
                    gurls.append(url)
                    if url != url0:
                        gurls.append(url0)
                    fileurls.extend(get_links(link))
                elif '?' not in link and not link[-4:] == '.php':
                    print(link)
                    fileurls.append(link)
            elif surl in link and '?' in link:
                fileurls.extend(get_links(link))
    except:
        import traceback
        # traceback.print_exc()
        print(url, '... error')
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
