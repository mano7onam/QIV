from bs4 import BeautifulSoup

import requests
import shutil
import io
import os
from PIL import Image  # https://pillow.readthedocs.io/en/4.3.x/

def download_image(url, image_file_path):
    r = requests.get(url, timeout=4.0)
    if r.status_code != requests.codes.ok:
        assert False, 'Status code error: {}.'.format(r.status_code)

    with Image.open(io.BytesIO(r.content)) as im:
        im.save(image_file_path)

    print('Image downloaded from url: {} and saved to: {}.'.format(url, image_file_path))
    

url = input("Enter a website to extract the URL's from: ")

# r = requests.get("http://" + url)
r = requests.get(url)

data = r.text

soup = BeautifulSoup(data)

links = soup.find_all('a')

for link in links:
    print("----------------------------")
    qhref = link.get('href')       
    if (qhref.find('-') == -1):
        continue
    print(qhref)
    qurl = url + qhref
    qr = requests.get(qurl)
    qdata = qr.text
    qsoup = BeautifulSoup(qdata)
    qlinks = qsoup.find_all('a')
    for qlink in qlinks:
        qqurl = qlink.get('href')
        if (qlink.get('href').find("jpg") != -1):
            fullqqurl = qurl + qqurl
            qqr = requests.get(fullqqurl, allow_redirects=True)
            open("../Data/" + qlink.get('href'), 'wb').write(qqr.content)
            print("Downloaded: " + fullqqurl)                        
    print("----------------------------")
