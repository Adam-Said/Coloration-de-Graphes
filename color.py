import random
import sys

#function to create a circle
def create_circle(node, color):
    return '<circle cx="' + str(node) + '" cy="' + str(node) + '" r="20" fill="' + color + '" />'

#function to create a line between two nodes
def create_line(node1, node2):
    return '<line x1="' + str(node1) + '" y1="' + str(node1) + '" x2="' + str(node2) + '" y2="' + str(node2) + '" style="stroke:rgb(0,0,0);stroke-width:2" />'



def generate_svg(nodes, edges, colors):
    # find the minimum and maximum x and y coordinates of the nodes
    min_x = min(x for x, y in nodes.values())
    min_y = min(y for x, y in nodes.values())
    max_x = max(x for x, y in nodes.values())
    max_y = max(y for x, y in nodes.values())

    # calculate the size of the SVG canvas based on the positions of the nodes
    width = max_x - min_x + 100
    height = max_y - min_y + 100

    # create the SVG header
    svg = '<?xml version="1.0" encoding="utf-8"?>\n'
    svg += f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}">\n'

    # define a color map
    color_map = {
        1: 'red',
        2: 'orange',
        3: 'yellow',
        4: 'green',
        5: 'blue',
        6: 'indigo',
        7: 'violet',
        8: 'purple',
        9: 'pink',
        10: 'brown',
        11: 'gray',
        12: 'black',
        13: 'white',
        14: 'maroon',
        15: 'olive',
        16: 'navy',
        17: 'teal',
        18: 'aqua',
        19: 'fuchsia',
        20: 'lime',
    }

    # draw the edges
    for edge in edges:
        node1, node2 = edge
        x1, y1 = nodes[node1]
        x2, y2 = nodes[node2]
        svg += f'  <line x1="{x1 - min_x + 50}" y1="{y1 - min_y + 50}" x2="{x2 - min_x + 50}" y2="{y2 - min_y + 50}" stroke="black" stroke-width="1" />\n'

    # draw the nodes
    for node, pos in nodes.items():
        x, y = pos
        color = 0
        if(colors[node]%20 == 0) :
            color = 20
        else :
            color = color_map[colors[node]%20]
        svg += f'  <circle cx="{x - min_x + 50}" cy="{y - min_y + 50}" r="10" fill="{color}" />\n'
        svg += f'  <text x="{x - min_x + 50}" y="{y - min_y + 50}" text-anchor="middle" alignment-baseline="central" font-size="12">{node}</text>\n'

    # close the SVG
    svg += '</svg>\n'

    return svg


def main():
    # parse the input file
    nodes = {}
    edges = []
    colors = {}
    with open(sys.argv[1]) as f:
        for line in f:
            parts = line.strip().split(' - ')
            node = int(parts[0])
            neighbors = [int(x) for x in parts[1].split()]
            color = int(parts[2])

            # assign a random position to the node
            nodes[node] = (random.randint(0, 100*node), random.randint(0, 100*node))

            # add the edges
            for neighbor in neighbors:
                if neighbor != 0:
                    edges.append((node, neighbor))

            # store the color of the node
            colors[node] = color

    # generate the SVG
    svg = generate_svg(nodes, edges, colors)

    # write the SVG to a file
    with open('graph.svg', 'w') as f:
        f.write(svg)

    print("Fichier graph.svg généré avec succès !")


if __name__ == '__main__':
    main()
