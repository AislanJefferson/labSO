class NRUPolicy:
    def __init__(self):
        self.list = []

    def put(self, frameId):
        frame = {'isRead': False, 'isWrite': False, 'frameId': frameId, 'count': 0}
        self.list.append(frame)


    def evict(self):
        wasRemoved = False
        frame_id = -1
        for frame in self.list:
            if ((not frame['isRead']) & (not frame['isWrite'])):
                frame_id = frame['frameId']
                self.list.remove(frame)
                wasRemoved = True
                break

        if (not wasRemoved):    
            for frame in self.list:
                if ((not frame['isRead']) & frame['isWrite']):
                    frame_id = frame['frameId']
                    self.list.remove(frame)
                    wasRemoved = True
                    break
        if (not wasRemoved):
            for frame in self.list:
                if (frame['isRead'] & (not frame['isWrite'])):
                    frame_id = frame['frameId']
                    self.list.remove(frame)
                    wasRemoved = True
                    break
        if (not wasRemoved):    
            for frame in self.list:
                if (frame['isRead'] & frame['isWrite']):
                    frame_id = frame['frameId']
                    self.list.remove(frame)
                    break

        return frame_id


    def clock(self):
        for frame in self.list:
            if (frame['isRead']):
                frame['isRead'] = False

    def access(self, frameId, isWrite):
        for frame in self.list:
            if(frameId == frame['frameId']):
                frame['isRead'] = True
                if (isWrite):
                    frame['isWrite'] = True

