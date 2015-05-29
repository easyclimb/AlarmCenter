# -*- coding: utf-8 -*-

#address = ''
#city_code = ''
#x = 0.0
#y = 0.0

def generate_faile_result(status):
    pass

def func_locate_by_ip(default_ip = ''):
    try:
        if len(default_ip) > 0: default_ip = '&ip=%s' % default_ip
        else:default_ip = ''
        url = 'http://api.map.baidu.com/location/ip?ak=dEVpRfhLB3ITm2Eenn0uEF3w&coor=bd09ll' + default_ip
        print url
        import urllib
        page = urllib.urlopen(url)
        data = page.read()
        print data
        import json
        dic = json.loads(data)
        status = dic[u'status']
        if status != 0:
            return (status, -1, -1, -1)
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
        #     print province, city, '%d' % city_code address
#         print address.__class__   
#         print u'address: ', address, 'x:' + point[u'x'], 'y:' + point[u'y']
#         print u'utf16', address.encode(u'utf-16')
#         print u'utf8', address.decode(u'utf-8')
#         f = open("e:/err.txt", "w")
#         f.write(address)
#         f.write("\r\n")
#         f.close()
#         f = open("e:/err.txt", "w")
#         address = address.encode('utf-8')
#         f.write(address)
#         f.close()
        return (status, address, int(city_code), float(x), float(y))
    except Exception as e:
        f = open("e:/err.txt", "w")
        f.write(str(e))
        f.close()
        return (88888, u'err:' + str(e), 1,2,3)
    else:
        return (99999, u"abc",1,2,4)
    
#def get_citycode():
#    return city_code
#
#def get_address():
#    return address

#def get_x():
#    return x

#def get_y():
#    return y


def test():
    return 'abc'

if __name__ == "__main__":
    (status,addr,code,x,y)= func_locate_by_ip()
    print '++++++++++++++++++++++++++main++++++++++++++++++++++++++++++++'
    #print addr
    print 'status', status
    print 'addr', addr.encode('utf-8')
    print 'x', x
    print 'y', y
    #'8.8.8.8'