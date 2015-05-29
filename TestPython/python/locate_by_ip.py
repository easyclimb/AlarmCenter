# -*- coding: utf-8 -*- 

address = ''
city_code = ''
x = 0.0
y = 0.0

def locate_by_ip(default_ip = ''):
    if len(default_ip) > 0: default_ip = '&ip=%s' % default_ip
    else:default_ip = ''
    url = 'http://api.map.baidu.com/location/ip?ak=dEVpRfhLB3ITm2Eenn0uEF3w' + default_ip
    print url
    import urllib
    page = urllib.urlopen(url)
    data = page.read()
    print data
    import json
    dic = json.loads(data)
    #print dic
    #addr = dic[u'address']
    addr_detail = dic[u'content'][u'address_detail']
    #province = addr_detail[u'province']
    #city = addr_detail[u'city']
    city_code = addr_detail[u'city_code']
    #print 'addr:', addr
    ##print 'province:', province
    ##print 'city:', city
    address = dic[u'content'][u'address']
    point = dic[u'content'][u'point']
    x = float(point[u'x'])
    y = float(point[u'y'])
    #     print province, city, '城市代码：%d' % city_code, address
    #print address, 'x:' + point[u'x'], 'y:' + point[u'y']
    return 1
    
def get_citycode():
    return city_code

def get_address():
    return address

def get_x():
    return x

def get_y():
    return y

if __name__ == "__main__":
    locate_by_ip()
    #'8.8.8.8'