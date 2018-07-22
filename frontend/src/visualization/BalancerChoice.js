import React, { Component } from 'react';

export default class extends Component {

    render(){
        return (
        <div class="list-group balancerChoice">
            <a href="#" class="list-group-item list-group-item-action active" data-toggle="list" role="tab">Naive Balancer</a>
            <a href="#" class="list-group-item list-group-item-action" data-toggle="list" role="tab">Prediction load balancer</a>
            <a href="#" class="list-group-item list-group-item-action disabled" data-toggle="list" role="tab">Morbi leo risus</a>
            <a href="#" class="list-group-item list-group-item-action disabled" data-toggle="list" role="tab">Porta ac consectetur ac</a>
        </div>);
    }
}