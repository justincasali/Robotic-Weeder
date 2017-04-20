scale = 13.75 / 30

x_origin = 640 / 2
y_origin = 480 / 2

def coord_scale(x_array, y_array):

    x_return = list()
    y_return = list()

    for x in x_array:
        x_return.append(round((x - x_origin) * (1 + scale) + x_origin))

    for y in y_array:
        y_return.append(round((y - y_origin) * (1 + scale) + y_origin))

    return x_return, y_return

x_array = [101, 539]
y_array = [76, 404]

print(coord_scale(x_array, y_array))
