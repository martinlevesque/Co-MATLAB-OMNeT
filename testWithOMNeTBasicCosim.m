function testWithOMNeTBasicCosim()

    

    tcpSocket = tcpip('127.0.0.1', 25000, 'NetworkRole', 'client');

    set(tcpSocket, 'InputBufferSize', 3000000);  % Set size of rx buffer.
    set(tcpSocket, 'OutputBufferSize', 3000000); % Set size of tx buffer.
    
    fopen(tcpSocket);
    set(tcpSocket,'Timeout',10000);
    

    % Start OMNeT
    disp('Start Omnet:0.0');
    fprintf(tcpSocket, 'Start Omnet:0.0');


    time = 1.0;
    msgId = 1;

    while time <= 10

        % Send msgs in a short period of time
         sendMsg(tcpSocket, msgId*100, 'HAN3', 'DRR3', 'daaaaapayloadaaaaapayloadaaaaap', time + 0.0002);


        % then grant (time advance)
        grant(tcpSocket, time + 0.5);

        % and get response
        res = fscanf(tcpSocket, '%s');

        disp(strcat('received ', res));

        % Analyze messages received up to time+0.5:
        receivedMsgs = strsplit(res, ';');

        for message = receivedMsgs
            % fields <id> <time received> <delay>

            fields = strsplit(message{:}, ',');

            if size(fields, 2) < 3
                continue
            end

            id = fields(1);
            timeReceived = fields(2);
            delay = fields(3);

            % do something
        end

        time = time + 0.5;
        msgId = msgId + 1;
        disp('pausing...');
    end

    fclose(tcpSocket);
end

function grant(tcpSock, time)
    msg = strcat('Event Time Grant:', num2str(time));
    disp(strcat('Sending ', msg));
    fprintf(tcpSock, msg);
end

function sendMsg(tcpSock, msgId, source, dest, payload, time)
    msg = strcat('<msgComRequestFromRTI id="', num2str(msgId), '" source="', source, '" destination="', dest, '" payload="', payload, '" time="', num2str(time), '"/>');
    disp(strcat('Sending ', msg));
    fprintf(tcpSock, msg);
end
