#!/usr/bin/env python3

# Make lookup table for Cinth and make life easier.


r = 86207 # sample rate

def calc_phases(length=pow(2,16), show_error=False):

  res = []
  error = 0
  freq_orig = 0

  for i in range(25):

    j = i - 9
    f = 440 * pow(2, j / 12)
    phase = length * f / r

    rounded = round(phase)
    diff = abs(phase - rounded)
    error = max(error, diff)

    res.append(rounded)

  if show_error:
    freq_error = error * r / length
    print('maximum error: {}% which is {}Hz.'.format(round(error,2), round(freq_error,2)))

  return res

def pretty_print(table):

  table = [hex(i) for i in table]

  longest = 0
  for n in table:
    longest = max(longest, len(n))

  for i in range(len(table)):
    if len(table[i]) != longest:

      diff = longest - len(table[i])
      table[i] = table[i].replace('0x','0x' + diff * '0')

  table = str(table)

  for c in '[]\'':
    table = table.replace(c,'')

  j = 0
  for i in range(len(table)):

    if table[i] == ' ':
      j += 1

      if (j) % 5 == 0:
        # replace table[i] with newline
        table = table[:i-1] + '\n' + table[i:]

  table = table.split('\n')
  for i in range(len(table)):
    table[i] = ' ' + table[i]
    if i + 1 != len(table):
      table[i] += ','
 
  table = '\n'.join(table)

  print("\nunsigned int phasediff_table[] = {{\n {}\n}};\n".format(table))


if __name__ == '__main__':

  try:
    from sys import argv as args
    length = int(args[1])
    table = calc_phases(length)
  except Exception:
    table = calc_phases()

  pretty_print(table)
