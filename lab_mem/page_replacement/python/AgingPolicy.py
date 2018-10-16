from NRUPolicy import NRUPolicy

class AgingPolicy(NRUPolicy):
    def evict(self):
        n = 256 #count maximo eh 255
        frame_to_remove = {}
        for frame in self.list:
            
            if (frame['count'] < n):
                n = frame['count']
                frame_to_remove = frame

        self.list.remove(frame_to_remove)
        return frame_to_remove['frameId']

    def clock(self):
        for frame in self.list:
            bit = 0
            if (frame['isRead']):
                bit = 1
                frame['isRead'] = False

            frame['count'] = shitf_count(frame['count'], bit)

    def access(self, frameId, isWrite):
        for frame in self.list:
            if(frameId == frame['frameId']):
                frame['isRead'] = True


    def shitf_count(count, bit):
        if ((count == 0) & (bit == 1)):        
            new_count = 128
        else:
            new_count = count * (bit) + (count >> 1)

        if (new_count > 255):
            new_count = 255

        return new_count

    def shitf_count(count, bit):
        add_bit = bit
        if (bit == 1):        
            add_bit = 128

        return add_bit | (count >> 1)

