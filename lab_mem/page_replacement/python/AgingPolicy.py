from NRUPolicy import NRUPolicy

class AgingPolicy(NRUPolicy):
    def evict(self):
        n = 256
        frame_to_remove = {}
        for frame in self.list:
            count = self.bin_to_dec(frame)
            if (count < n):
                n = count
                frame_to_remove = frame
                
        self.list.remove(frame_to_remove)
        return frame_to_remove['frameId']

    def clock(self):
        for frame in self.list:
            bit = '0'
            if (frame['isRead']):
                bit = '1'
                frame['isRead'] = False

            new_count = frame['count'][0:7]
            new_count = bit + new_count
            frame['count'] = new_count

    def access(self, frameId, isWrite):
        for frame in self.list:
            if(frameId == frame['frameId']):
                frame['isRead'] = True

    def bin_to_dec(self, frame):
        count_bin = frame['count']
        count_int = 0
        for i in range(7, -1, -1):
            if (count_bin[i] == '1'):
                count_int += 2 ** (7 - i)

        return count_int

    def shitf_count(count, bit):
        if (count == 0):
            if (bit == 1):
                new_count = 128
        else:
            new_count = count * (bit) + (count >> 1)

        if (new_count > 255):
            new_count = 255

        return new_count

