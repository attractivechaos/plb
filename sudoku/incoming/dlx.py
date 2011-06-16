# dlx.py
# Copyright (c) 2006,2008 Antti Ajanki <antti.ajanki@iki.fi>

# DLXMatrix class is used to store and solve an exact cover problem
# with help of Dancing Links [1] technique by Donald Knuth. The
# problem can be stated as an attempt to leave out rows of a 0/1
# matrix until remaining matrix has exactly one 1 in each column.
# Knuth proposes a fast solution technique based on clever trick with
# double linked list.
#
# This implementation will return row numbers of the solution instead
# column indexes (like in the Knuth's paper).
#
# [1] Donald E Knuth, Dancing links, preprint, available at
# http://www-cs-faculty.stanford.edu/~knuth/preprints.html

# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.

ROOTNODE = 0

# Node's attributes
# LEFT, RIGHT, UP and DOWN are identifiers of corresponding neighbors
# INDEX is the (row) index of the node (None in the header nodes)
# COLUMN/COUNT is column index on a regular node and count of nodes on
# a header node
LEFT   = 0
RIGHT  = 1
UP     = 2
DOWN   = 3
COLUMN = 4
INDEX  = 5
COUNT  = 5


class DLXMatrix:
    def __init__(self):
        self.cursolution = []
        self.solutioncallback = None
        self.nodes = [[0, 0, None, None, None, None]]

    def walknodes(self, firstnode, direction):
        """Generator for iterating over all nodes in given direction
        (not including firstnode)."""
        nodetable=self.nodes
        n = nodetable[firstnode][direction]
        while n != firstnode:
            yield n
            n = nodetable[n][direction]

    def constructmatrix(self, ones, initialsolution=[]):
        """Construct the (sparse) DLX matrix based on list 'ones'. The
        first component in the list elements is row index (which will
        be returned by solve) and the second component is list of
        column indexes of ones in given row.

        'initialsolution' is list of row indexes that are required to
        be part of the solution. They will be removed from the matrix."""
        self.cursolution = []
        # LEFT, RIGHT, UP, DOWN, COLUMN, INDEX/COUNT
        self.nodes = [[ROOTNODE, ROOTNODE, None, None, None, None]]

        # optimization: local variables are faster
        nodetable = self.nodes
        ones.sort()
        pruneNodes = []
        headers = [ROOTNODE] # indexes of header nodes for faster access
        for r in ones:
            curRow = r[0]  # row index
            columns = r[1] # column indexes
            if len(columns) == 0: continue
            columns.sort()

            # Do we need more headers?
            while len(headers) <= columns[-1]:
                lastheader = headers[-1]
                newind = len(nodetable)
                nodetable.append([lastheader, ROOTNODE, newind, newind, None, 0])
                nodetable[ROOTNODE][LEFT] = newind
                nodetable[lastheader][RIGHT] = newind
                headers.append(newind)

            # Add new nodes to indexes newind..newind+len(columns)-1
            # LEFT and RIGHT links can be calculated when created the
            # node, only UP, DONW and COUNT have to be updated when
            # adding new nodes
            newind = len(nodetable)
            for i, c in enumerate(columns):
                h = headers[c]
                l = newind + ((i-1) % len(columns))
                r = newind + ((i+1) % len(columns))
                nodetable.append([l, r, nodetable[h][UP], h, h, curRow])
                nodetable[nodetable[h][UP]][DOWN] = newind+i
                nodetable[h][UP] = newind+i
                nodetable[h][COUNT] += 1

            if curRow in initialsolution:
                pruneNodes.append(newind)


        # Remove columns that are required to be in the solution
        for n in pruneNodes:
            self.cursolution += [nodetable[n][INDEX]]
            self.covercolumn(nodetable[n][COLUMN])
            for j in self.walknodes(n, RIGHT):
                self.covercolumn(nodetable[j][COLUMN])

    def covercolumn(self, c):
        nodetable = self.nodes
        nodetable[nodetable[c][RIGHT]][LEFT] = nodetable[c][LEFT]
        nodetable[nodetable[c][LEFT]][RIGHT] = nodetable[c][RIGHT]
        for i in self.walknodes(c, DOWN):
            for j in self.walknodes(i, RIGHT):
                nodetable[nodetable[j][DOWN]][UP] = nodetable[j][UP]
                nodetable[nodetable[j][UP]][DOWN] = nodetable[j][DOWN]
                nodetable[nodetable[j][COLUMN]][COUNT] -= 1

    def uncovercolumn(self, c):
        nodetable = self.nodes
        for i in self.walknodes(c, UP):
            for j in self.walknodes(i, LEFT):
                nodetable[nodetable[j][DOWN]][UP] = j
                nodetable[nodetable[j][UP]][DOWN] = j
                nodetable[nodetable[j][COLUMN]][COUNT] += 1
        nodetable[nodetable[c][RIGHT]][LEFT] = c
        nodetable[nodetable[c][LEFT]][RIGHT] = c

    def dosearch(self):
        """Internal. The actual recursive searching function."""
        nodetable = self.nodes # optimization: local variables are faster

        if nodetable[ROOTNODE][RIGHT] == ROOTNODE:
            if self.solutioncallback is not None:
                self.solutioncallback(self.cursolution)
                return None
            else:
                return self.cursolution
        a = None
        c = nodetable[ROOTNODE][RIGHT]
        maxcount = nodetable[nodetable[ROOTNODE][RIGHT]][COUNT]
        for j in self.walknodes(ROOTNODE, RIGHT):
            if nodetable[j][COUNT] < maxcount:
                c = j
                maxcount = nodetable[j][COUNT]
        self.covercolumn(c)
        for r in self.walknodes(c, DOWN):
            self.cursolution += [nodetable[r][INDEX]]
            for j in self.walknodes(r, RIGHT):
                self.covercolumn(nodetable[j][COLUMN])
            a = self.dosearch()
            # Returning here leaves the matrix in inconsistent state,
            # but as the matrix is initialized by solve() anyway this
            # doesn't matter
            if a is not None: return a
            self.cursolution = self.cursolution[:-1]
            for j in self.walknodes(r, LEFT):
                self.uncovercolumn(nodetable[j][COLUMN])
        self.uncovercolumn(c)
        return None

    def solve(self, ones, initialsolution=[], callback=None):
        """Construct DLX matrix and solve exact cover problem. Returns
        list of row indexes of found solution or None none is found.
        
        If callback is given, tries to find all solutions and calls
        the callback with one argument once for every solution. The
        argument is list of row indexes. If callback is given this
        method always returns None."""
        self.constructmatrix(ones, initialsolution)
        self.solutioncallback=callback
        return self.dosearch()


if __name__ == '__main__':
    rows = [[1, [3,5,6]], [2, [1,4,7]], [3, [2,3,6]],
            [4, [1,4]], [5, [2,7]], [6, [4,5,7]]]
    m = DLXMatrix()
    print m.solve(rows)
